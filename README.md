grom_vs_adjust
======================

grom_vs_adjust は，HONDA GROM (海外名: MSX125, MSX125SF) のスピードパルス信号を調整するための Arduino ソフトウェアです．

スプロケット交換などにより生じた，実際のスピードとメーター表示上のスピードのずれを調整することを目的としています．

## 動作環境 ##

* HONDA GROM (JC75 で動作確認，JC61 は未確認)
* Arduino Pro Micro 5V, 16MHz 品 → [AliExpress の販売ページ](https://www.aliexpress.com/wholesale?catId=0&initiative_id=SB_20201026062514&SearchText=arduino+pro+micro+atmega32u4)

## 組み込み手順 ##

1. Arduino Pro Micro の開発環境を準備します →[参考](https://www.google.com/search?q=arduino+pro+micro+%E9%96%8B%E7%99%BA%E7%92%B0%E5%A2%83&rlz=1C1CHBD_jaJP923JP924&oq=arduino+pro+micro+%E9%96%8B%E7%99%BA%E7%92%B0%E5%A2%83&aqs=chrome..69i57j0i333l4.12162j0j7&sourceid=chrome&ie=UTF-8)
1. 本プログラム 1行目の `SCALE` を修正し，標準の速度表示の何倍の値をメーターに表示するかを浮動小数で記述します．1.0 未満の値を指定することもできます．
1. 本プログラムを Arduino に焼きます．
1. バイクの車速パルスのギボシ端子を特定してください．そのギボシ端子に割り込ませるように Arduino を接続します．Arduino の 4番ピンを車速センサー側に，5番ピンをメーター側に接続してください．

## 免責事項等 ##

本ソフトウェアを使用することにより生じたいかなる損害も，私は責任を負いません (バイクの故障，事故による傷害・死亡等)．本ソフトウェアは使用者自身の責任において使用してください．
