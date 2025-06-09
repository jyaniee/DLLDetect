import pandas as pd
from analyze_dll import analyze_dll

# 1) 원본 읽기
df = pd.read_csv('analyzed_dlls_cleaned.csv')

# 2) 악성 DLL 특징 뽑기
mal_path = r'C:\Users\daesung\Desktop\inject_test_package_fixed\InjectTest\build\Desktop_Qt_6_9_0_MinGW_64_bit-Debug\libtest_malicious.dll'
feat = analyze_dll(mal_path)
if feat is None:
    raise RuntimeError("malicious DLL 분석에 실패했습니다.")

# 3) 레이블 붙이고 DataFrame 으로
feat['label'] = 1
mal_df = pd.DataFrame([feat])

# 4) 합치고 저장
new_df = pd.concat([df, mal_df], ignore_index=True)
new_df.to_csv('analyzed_dlls_with_mal.csv', index=False)
print(f"악성 샘플 추가 완료: {len(new_df)} rows")
