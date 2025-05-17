import pefile
import os

def analyze_dll(filepath):
    try:
        pe = pefile.PE(filepath)
        
        # 파일 크기
        file_size = os.path.getsize(filepath)
        
        # 섹션 수
        num_sections = len(pe.sections)
        
        # export 함수 수
        try:
            export_count = len(pe.DIRECTORY_ENTRY_EXPORT.symbols)
        except AttributeError:
            export_count = 0
        
        # 엔트로피 평균
        entropy_list = [section.get_entropy() for section in pe.sections]
        avg_entropy = sum(entropy_list) / len(entropy_list) if entropy_list else 0
        
        return {
            'file_size': file_size,
            'num_sections': num_sections,
            'export_count': export_count,
            'avg_entropy': avg_entropy
        }
    
    except Exception as e:
        print(f"Error analyzing {filepath}: {e}")
        return None

# 테스트
dll_path = r"C:\Windows\System32\kernel32.dll"  # 테스트할 DLL 파일 경로
features = analyze_dll(dll_path)

if features:
    print(features)
