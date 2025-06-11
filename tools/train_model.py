import pandas as pd
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score
import joblib

# CSV 파일 읽기
data = pd.read_csv('analyzed_dlls_with_mal.csv')
data['label'] = data['label'].astype(int)


# X(입력 특징)와 y(출력 레이블) 분리
# 파일 크기, 섹션 수, export 수, 엔트로피만 학습 대상으로 사용
X = data[['file_size', 'num_sections', 'export_count', 'avg_entropy']]

# y는 일단 '모든 데이터가 정상(0)'이라고 가정
# (나중에 악성 DLL 데이터 추가하면 바꿀 수 있음)
y = data['label']

# 학습용/테스트용 데이터셋 분리
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# 모델 학습
model = RandomForestClassifier()
model.fit(X_train, y_train)

# 테스트 정확도 출력
y_pred = model.predict(X_test)
print(f"Accuracy: {accuracy_score(y_test, y_pred)}")

# 모델 저장
joblib.dump(model, 'dll_classifier.pkl')

print("모델 저장 완료: 'dll_classifier.pkl")
