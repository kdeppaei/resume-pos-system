# Resume POS System V3.0

[![C build and tests](https://github.com/kdeppaei/resume-pos-system/actions/workflows/ci.yml/badge.svg)](https://github.com/kdeppaei/resume-pos-system/actions/workflows/ci.yml)

這是一個使用 **C 語言**開發的模組化 POS 與庫存管理系統。V3 將原本約 1,500 行的單一 `main.c` 重構成多個 `.c/.h` 模組，並加入自動化邏輯測試與 GitHub Actions 持續整合。

## 專案重點

- Dev-C++／舊版 GCC 相容，採用 GNU90 寫法
- 商品、輸入、儲存、交易、結帳與報表責任分離
- 函式透過 `Database *` 傳遞狀態，不依賴可任意修改的全域資料庫
- 自行實作 **Merge Sort**，依商品編號與營收排序，時間複雜度為 `O(n log n)`
- 結帳前建立完整資料庫快照，寫檔失敗時回滾
- 使用 temporary file + backup file 降低資料毀損風險
- 商品採軟刪除，可封存及恢復，歷史收據保留商品快照
- 交易紀錄使用固定容量循環緩衝區，控制記憶體上限
- 可匯出商品與交易 CSV 報表
- GitHub Actions 會在每次推送與 Pull Request 自動建置並執行測試

## 專案結構

```text
pos_resume_v3_modular/
├─ include/
│  ├─ pos_types.h       共用常數與資料結構
│  ├─ input.h           輸入驗證介面
│  ├─ core.h            查詢、促銷、排序與共用邏輯
│  ├─ storage.h         資料載入與安全儲存
│  ├─ catalog.h         商品管理
│  ├─ transaction.h     交易快照與循環緩衝區
│  ├─ checkout.h        結帳流程
│  └─ reports.h         報表與 CSV
├─ src/
│  ├─ main.c
│  ├─ input.c
│  ├─ core.c
│  ├─ storage.c
│  ├─ catalog.c
│  ├─ transaction.c
│  ├─ checkout.c
│  └─ reports.c
├─ tests/
│  └─ test_logic.c
├─ ResumePOS.dev        Dev-C++ 專案檔
├─ Makefile
├─ build_windows.bat
└─ run_tests.bat
```

## 功能

1. 新增商品
2. 依編號列出商品
3. 商品編號查詢與名稱模糊搜尋
4. 修改名稱、價格、庫存與促銷
5. 商品進貨
6. 購物車與結帳
7. 商品封存與恢復
8. 買 X 送 Y
9. 銷售摘要與營收排行
10. 低庫存報表
11. 交易紀錄與收據重印
12. CSV 匯出
13. 主資料檔損壞時讀取備份

## Dev-C++ 執行

1. 將 ZIP 完整解壓縮，例如 `C:\POS_V3\`。
2. 使用 Dev-C++ 開啟 `ResumePOS.dev`，不要只開某一個 `.c`。
3. 選擇「執行 → 編譯並執行」。

若 `.dev` 因 Dev-C++ 版本差異無法載入，可直接雙擊：

```text
build_windows.bat
```

## GCC 編譯

```bash
gcc -std=gnu90 -Wall -Wextra -Iinclude src/main.c src/input.c src/core.c src/storage.c src/catalog.c src/transaction.c src/checkout.c src/reports.c -o ResumePOS.exe
```

或執行：

```bash
make
```

## 執行測試

Windows：

```text
run_tests.bat
```

GCC／Make：

```bash
make test
```

目前測試包含：

- 買 2 送 1 邊界值
- 大小寫不敏感商品查詢
- 商品編號排序
- 交易循環緩衝區覆蓋行為
- 收據商品快照不受後續改名影響

## 資料檔

```text
pos_data_v2.dat
pos_data_v2.bak
```

V3 保持與 V2 相同的資料結構，因此可以沿用 V2 資料檔。請先備份再搬移。

## 履歷描述範例

> 使用 C 語言重構 POS 與庫存管理系統，將 1,500 行單檔程式拆分為輸入、商品、交易、結帳、儲存與報表模組；自行實作 O(n log n) Merge Sort、固定容量循環交易緩衝區、交易快照、軟刪除及 temporary/backup 原子化儲存流程，並以 assert 建立促銷、查詢、排序與交易保存測試。

## License

[MIT](LICENSE)
