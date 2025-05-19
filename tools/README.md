# Tools

이 폴더는 DLLDetect 프로젝트의 보조 도구들을 모아놓은 곳입니다.

- `analyze_folder.py` :  
  지정한 폴더 안의 모든 DLL 파일을 분석하여,
  파일 크기, 섹션 수, export 함수 수, 평균 엔트로피를 추출하고
  결과를 CSV 파일(`analyzed_dlls.csv`)로 저장합니다.

- `analyzed_dlls.csv` :  
  위 Python 스크립트를 이용해 수집된 DLL 특징 데이터셋입니다.  
  머신러닝 학습용으로 사용됩니다.

## 주의
- 본 폴더는 데이터 준비를 위한 용도이며,  
  최종 DLLDetect 프로그램 구동에는 필요하지 않습니다.

- `train_model.py` :
  analyzed_dlls.csv 파일을 읽어 RandomForestClassifier 모델을 학습시키고,
  학습된 모델을 dll_classifier.pkl로 저장합니다.

- `dll_classifier.pkl` :
  학습된 머신러닝 모델 파일입니다.
  추후 DLLDetect 프로그램에서 DLL 정상/비정상 판별에 사용됩니다.
