import pefile
import os
import csv

def analyze_dll(filepath):
    try:
        pe = pefile.PE(filepath)
        
        file_size = os.path.getsize(filepath)
        num_sections = len(pe.sections)
        
        try:
            export_count = len(pe.DIRECTORY_ENTRY_EXPORT.symbols)
        except AttributeError:
            export_count = 0
        
        entropy_list = [section.get_entropy() for section in pe.sections]
        avg_entropy = sum(entropy_list) / len(entropy_list) if entropy_list else 0
        
        return {
            'filepath': filepath,
            'file_size': file_size,
            'num_sections': num_sections,
            'export_count': export_count,
            'avg_entropy': avg_entropy
        }
    
    except Exception as e:
        print(f"Error analyzing {filepath}: {e}")
        return None

def analyze_folder_and_save(folder_path, output_csv):
    results = []
    
    for filename in os.listdir(folder_path):
        if filename.lower().endswith('.dll'):
            full_path = os.path.join(folder_path, filename)
            features = analyze_dll(full_path)
            if features:
                results.append(features)
    
    # CSV로 저장
    with open(output_csv, mode='w', newline='', encoding='utf-8') as f:
        writer = csv.DictWriter(f, fieldnames=['filepath', 'file_size', 'num_sections', 'export_count', 'avg_entropy'])
        writer.writeheader()
        for data in results:
            writer.writerow(data)

# 사용 예시
folder_to_analyze = r"C:\Windows\System32"  # 분석할 DLL 폴더 경로
output_csv_path = r"analyzed_dlls.csv"       # 저장할 CSV 파일 이름

analyze_folder_and_save(folder_to_analyze, output_csv_path)
