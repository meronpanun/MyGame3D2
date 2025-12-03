import csv
import os
import sys

file_path = r'c:\Users\由留部 瑛人.FICPCROOMB14\Documents\GitHub\MyGame3D2\3DShooting\3DShooting\data\CSV\TutorialStageCollisionData.csv'
log_file = 'debug_log.txt'

print(f"Debugging file: {file_path}")

with open(log_file, 'w', encoding='utf-8') as log:
    try:
        with open(file_path, 'r', newline='', encoding='utf-8-sig') as infile:
            reader = csv.reader(infile)
            header = next(reader)
            log.write(f"Header: {header}\n")
            
            for i, row in enumerate(reader):
                if i >= 30: break # Check first 30 rows
                
                log.write(f"Row {i}: {row}\n")
                if len(row) > 0:
                    name = row[0].strip()
                    log.write(f"  Name: '{name}' (Match UNIConcrete: {name == 'UNIConcrete'})\n")
                    
                    if name == 'UNIConcrete':
                        for idx in [2, 5, 8]:
                            if idx < len(row):
                                val_str = row[idx]
                                try:
                                    val = float(val_str)
                                    log.write(f"    Idx {idx}: '{val_str}' -> {val} ( > 50: {val > 50.0})\n")
                                except ValueError:
                                    log.write(f"    Idx {idx}: '{val_str}' -> ValueError\n")

    except Exception as e:
        log.write(f"Error: {e}\n")
        print(f"Error: {e}")

print("Debug finished. Check debug_log.txt")
