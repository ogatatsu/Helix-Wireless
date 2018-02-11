# Helix-Wireless

[Helix](https://github.com/MakotoKurauchi/helix)に[BL652-breakout](https://github.com/ogatatsu/BL652-breakout)
を載せてワイヤレスに改造した物。

## 仕組み
![1](./images/1.jpg)

## ビルド方法
- BL652-breakoutには予めFeather nRF52のブートローダーを書き込んでおく。

  参考URL
  
  https://learn.adafruit.com/bluefruit-nrf52-feather-learning-guide/flashing-the-bootloader
  https://qiita.com/ogatatsu@github/items/37f4888c885c4e287df1

- BL652-breakoutをHelix PCBの白枠線内とは逆の方に刺す。

- ArduinoをインストールしてボードマネージャーからFeather nRF52用の物をインストールしておく。

  参考URL

  https://learn.adafruit.com/bluefruit-nrf52-feather-learning-guide/arduino-bsp-setup

- ArduinoのライブラリマネージャからBounce2ライブラリをインストールする。
- 右側のボードにはSlave用のファームウェア、左側にはMaster用のファームウェアを書き込む。

## キーカスタマイズ
