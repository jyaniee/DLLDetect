from flask import Flask, request, jsonify
import joblib
import numpy as np
import pandas as pd
from analyze_dll import analyze_dll  # 🔥 DLL 특징 추출 함수 import
import base64                      #  <--- Base64 디코딩을 위해 추가
import os                          #  <--- 임시 파일 생성을 위해 추가

app = Flask(__name__)

#  사전에 학습된 모델 불러오기
model = joblib.load('dll_classifier.pkl')


# [수정됨] Base64로 인코딩된 여러 파일의 '내용'을 받음
@app.route('/bulk_predict', methods=['POST'])
def bulk_predict():
    data = request.json
    files_list = data.get('files') #  C++에서 보낸 'files' 배열을 받음

    if not files_list:
        return jsonify({'error': 'Missing files list'}), 400

    results = []

    for file_data in files_list:
        base64_content = file_data.get('file_content')
        file_name = file_data.get('file_name', 'temp_dll.dll') # (C++에서 보낸 파일 이름)

        if not base64_content:
            results.append({
                'dll_path': file_name, # C++이 파일 이름을 보내주므로 dll_path 대신 사용
                'error': 'Missing file_content'
            })
            continue

        temp_file_path = os.path.join("/tmp", file_name) # Linux 서버의 임시 폴더

        try:
            # 1. Base64 문자열을 디코딩하여 파일 바이트로 복원
            dll_bytes = base64.b64decode(base64_content)

            # 2. 임시 파일로 저장
            with open(temp_file_path, "wb") as f:
                f.write(dll_bytes)

            # 3. 임시 파일 경로로 분석 수행
            features = analyze_dll(temp_file_path)
            if not features:
                results.append({
                    'dll_path': file_name,
                    'error': 'DLL 분석 실패'
                })
                continue

            # 4. DataFrame 변환 및 예측 (이 부분은 원본 코드와 거의 동일)
            input_data = pd.DataFrame([features])
            prediction = int(model.predict(input_data)[0])

            results.append({
                'dll_path': file_name,
                'prediction': prediction
            })

        except Exception as e:
            results.append({
                'dll_path': file_name,
                'error': f'서버 내부 오류: {str(e)}'
            })

        finally:
            # 5. (중요) 분석이 끝났으니 임시 파일을 항상 삭제
            if os.path.exists(temp_file_path):
                os.remove(temp_file_path)

    # bulk_predict의 최종 결과 반환
    return jsonify({'results': results})



@app.route('/predict', methods=['POST'])
def predict():
    data = request.json

    # 1. C++이 보낸 "file_content" (Base64 문자열)를 받습니다.
    base64_content = data.get('file_content')
    file_name = data.get('file_name', 'temp_dll.dll') # (C++에서 보낸 파일 이름)

    if not base64_content:
        return jsonify({'error': 'Missing file_content'}), 400

    temp_file_path = os.path.join("/tmp", file_name) # Linux 서버의 임시 폴더

    try:
        # 2. Base64 문자열을 다시 파일 내용(바이트)으로 디코딩합니다.
        dll_bytes = base64.b64decode(base64_content)

        # 3. EC2 서버(Linux)에 임시 파일로 저장합니다. (예: /tmp/combase.dll)
        with open(temp_file_path, "wb") as f:
            f.write(dll_bytes)

        # 4. 방금 저장한 "임시 파일의 경로"로 분석을 돌립니다.
        features = analyze_dll(temp_file_path)
        if not features:
            return jsonify({'error': f'DLL 분석 실패: {file_name}'}), 500

        # DataFrame으로 변환
        input_data = pd.DataFrame([features])

        prediction = int(model.predict(input_data)[0])

        return jsonify({'prediction': prediction})

    except Exception as e:
        # (혹시 모를 디코딩/파일 쓰기 오류 처리)
        return jsonify({'error': f'서버 내부 오류: {str(e)}'}), 500

    finally:
        # 5. (중요) 분석이 끝났으니 임시 파일을 항상 삭제합니다.
        if os.path.exists(temp_file_path):
            os.remove(temp_file_path)


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
