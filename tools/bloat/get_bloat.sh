nm -CSr --size-sort ../../bin/distribute/bin/Linux/x86_64/Release/StereoKitC.sym > Size_Linux_x64.txt
nm -CSr --size-sort ../../bin/distribute/bin/Android/arm64-v8a/Release/StereoKitC.sym > Size_Android_arm64-v8a.txt
python3 BloatReport.py Size_Linux_x64.txt > Report_Linux_x64.md
python3 BloatReport.py Size_Android_arm64-v8a.txt > Report_Android_arm64-v8a.md