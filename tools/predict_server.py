from flask import Flask, request, jsonify
import joblib
import numpy as np

app = Flask(__name__)

# 모델 로딩
model = joblib.load('dll_classifier.pkl')

@app.route('/predict', methods=['POST'])
def predict():
    data = request.json
    
    # DLL 특징값 받기
    try:
        file_size = data['file_size']
        num_sections = data['num_sections']
        export_count = data['export_count']
        avg_entropy = data['avg_entropy']
    except KeyError:
        return jsonify({'error': 'Missing input features'}), 400

    # 특징을 모델 입력 형태로 변환
    features = np.array([[file_size, num_sections, export_count, avg_entropy]])
    
    # 예측
    prediction = model.predict(features)
    
    # 결과 반환
    return jsonify({'prediction': int(prediction[0])})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
