# ELSFP 支援指令

`<lane>` 可填 `1`～`8` 或 `all`。雷射指令必須明確指定Lane。

| 指令 | 用途 |
| --- | --- |
| `get system status` | 列印所有 Task工作狀態 與 Stack運行狀態 |
| `get system version` | 顯示韌體版本 |
| `help` | 列出指令分類 |
| `help get` / `help set` | 列出查詢／設定對象 |
| `help get system` | 顯示 Task 狀態查詢用法 |
| `help get elsfp` | 顯示整體查詢用法 |
| `help get msm` | 顯示 MSM 查詢用法 |
| `help get laser` / `help set laser` | 顯示雷射指令用法 |
| `get system status` | 查詢所有 Task 工作狀態與 Stack 最低剩餘空間 |
| `get elsfp status` | 查詢模組、全部 8 Lane雷射、TEC 及故障資料 |
| `get msm status` | 查詢 MSM 狀態、廣播序號及電源模式 |
| `get laser <lane> status` | 查詢指定Lane的要求設定、實際狀態與量測資料 |
| `set laser <lane> enable <0或1>` | 設定停用／啟用要求 |
| `set laser <lane> bias <uA>` | 設定偏壓電流要求，單位 µA |
| `set laser <lane> power <uW>` | 設定光功率目標，單位 µW |
| `set laser <lane> apc <0或1>` | 設定 APC 停用／啟用要求 |

