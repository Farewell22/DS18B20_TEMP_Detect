# DS18B20_TEMP_Detect
使用了滑动窗口滤波来对测温进行平滑处理
# 开发环境
- STM32CUBEIDE/KEILV5
- CUBEMX 
# 文件目录
|-- 00_Reference  #参考的开源组件 感谢 multitimer和multibutton @0xabin
|   |-- MultiButton-master 
|   |   `-- MultiButton-master
|   `-- MultiTimer-main
|       `-- MultiTimer-main
|-- 01_Function_Map  # 功能描述图，有出入，没有实现菜单界面，后续考虑
|  
|   
|-- 02_Hardware
|   |-- 00_Ref
|   |   |-- 00_HDK
|   |   `-- SCH_Schematic1_1_2024-12-28.pdf  # 硬件原理图
|   |-- 01_Project
|   |   `-- ProProject_STM32F1_TEMP_2024-12-15.epro # 嘉立创工程
|   `-- BOM_Board1_1_Schematic1_1_2024-12-06.xlsx  # bom表
|-- 03_Firmware
|   |-- TEMP_SLIDING_WINDOW   #温度检测 滑动窗口滤波版本
|   |   |-- Core
|   |   |-- Debug
|   |   |-- Drivers
|   |   |-- MDK-ARM
|   |   |-- STM32F103C8TX_FLASH.ld
|   |   |-- TEMP.ioc
|   |   `-- TEMP.launch
|   `-- TEMP_averge  # 温度检测 时间均值滤波版本
|       |-- Core
|       |-- Debug
|       |-- Drivers
|       |-- MDK-ARM
|       |-- STM32F103C8TX_FLASH.ld
|       |-- TEMP.ioc
|       `-- TEMP.launch
|-- LICENSE
`-- README.md

