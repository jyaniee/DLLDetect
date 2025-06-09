import pandas as pd
import os
from analyze_dll import analyze_dll

# 1. 분석할 DLL 경로
dll_path = 'ac_esp.dll'

# 2. 기존 CSV 읽기
csv_file = 'analyzed_dlls_cleaned.csv'
if not os.path.exists(csv_file):
    raise FileNotFoundError(f"{csv_file}가 디렉터리에 없습니다.")

df = pd.read_csv(csv_file)

# 3. DLL feature 추출
feat = analyze_dll(dll_path)
if feat is None:
    raise RuntimeError(f"{dll_path}의 특징 추출에 실패했습니다.")

# 4. label 붙이기 (1=악성)
feat['label'] = 1

# 5. DataFrame에 합치기
df = pd.concat([df, pd.DataFrame([feat])], ignore_index=True)

# 6. 덮어쓰기 저장
df.to_csv(csv_file, index=False)
print(f"{dll_path} 특징이 {csv_file}에 추가되었습니다. 총 행: {len(df)}")
