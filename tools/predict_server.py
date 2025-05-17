from flask import Flask, request, jsonify
import joblib
import numpy as np
from analyze_dll import analyze_dll  # 🔥 DLL 특징 추출 함수 import

app = Flask(__name__)

# 🔧 사전에 학습된 모델 불러오기
model = joblib.load('dll_classifier.pkl')

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

    # 🔢 특징값을 모델 입력 형태로 변환
    input_data = np.array([[features['file_size'],
                            features['num_sections'],
                            features['export_count'],
                            features['avg_entropy']]])

    # 🤖 예측
    prediction = model.predict(input_data)

    # ✅ 예측 결과 반환 (0: 정상, 1: 비정상)
    return jsonify({'prediction': int(prediction[0])})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
