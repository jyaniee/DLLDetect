from flask import Flask, request, jsonify
import joblib
import numpy as np
import pandas as pd
from analyze_dll import analyze_dll  # 🔥 DLL 특징 추출 함수 import

app = Flask(__name__)

# 🔧 사전에 학습된 모델 불러오기
model = joblib.load('dll_classifier.pkl')


@app.route('/bulk_predict', methods=['POST'])
def bulk_predict():
    data = request.json
    dll_list = data.get('dll_list')

    if not dll_list:
        return jsonify({'error': 'Missing dll_list'}), 400

    features_list = []
    paths_with_features = []

    for dll_path in dll_list:
        features = analyze_dll(dll_path)
        if not features:
            # 분석 실패한 경우는 별도로 처리
            features_list.append(None)
            paths_with_features.append(dll_path)
            continue

        features_list.append(features)
        paths_with_features.append(dll_path)

    # 분석 성공한 것만 DataFrame으로 변환
    valid_features = [f for f in features_list if f]
    if not valid_features:
        return jsonify({'error': '모든 DLL 분석이 실패했습니다.'}), 500

    input_data = pd.DataFrame(valid_features)

    # 예측 수행
    predictions = model.predict(input_data)

    # 결과 정리
    results = []
    idx = 0
    for i, f in enumerate(features_list):
        if not f:
            results.append({
                'dll_path': paths_with_features[i],
                'error': 'DLL 분석 실패'
            })
        else:
            results.append({
                'dll_path': paths_with_features[i],
                'prediction': int(predictions[idx])
            })
            idx += 1

    return jsonify({'results': results})


@app.route('/predict', methods=['POST'])
def predict():
    data = request.json

    # 🔍 Qt에서 보내는 DLL 경로 받기
    dll_path = data.get('dll_path')
    if not dll_path:
        return jsonify({'error': 'Missing dll_path'}), 400

    # 🧪 DLL 특징 추출
    features = analyze_dll(dll_path)
    if not features:
        return jsonify({'error': 'DLL 분석 실패'}), 500

    # DataFrame으로 변환
    input_data = pd.DataFrame([features])

    prediction = int(model.predict(input_data)[0])
    return jsonify({'prediction': prediction})


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
