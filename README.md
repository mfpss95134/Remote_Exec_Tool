# Remote_Exec_Tool


- 目前狀態：
	可以正常執行。
- 後續改進：
	- 之後改成 `fork` + `exec` 執行指令：目前使用 `popen` 是因為取得執行結果比較簡單。
	- 新增中斷連線功能：中斷特定連線
	- 新增重新連線功能：讓 client 斷線後會自動連回 server，不會 lose connection
 	- 新增群發功能：同時對多個 client 下指令執行

<div align="center">
<img src="https://raw.githubusercontent.com/mfpss95134/Remote_Exec_Tool/refs/heads/main/demo.png">
<div align="left">
<br>
