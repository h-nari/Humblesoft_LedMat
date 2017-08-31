# HSES-LMC1

このページでは HSES-LMC1のファームウェア(表示用サブCPUのプログラム)の
更新方法について説明します。

## firmwareの更新方法

ファームウェアの更新は、Arduino用のプログラムで行います。
Humblesoft_LedMatライブラリを最新のものにし、以下の手順で
行ってください。

* arduino-IDEを起動する。
* メニューバーで ファイル → スケッチの例 → Humblesoft_LedMat → firmware_updateを開く
* ファイル → 名前をつけて保存 で、適当な場所に保存
* 9行目、10行目に ご使用のWiFi環境のSSIDとパスワードを入力
* ツールメニューをHSES-LMC1用に設定
* HSES-LMC1をPCに接続、電源を入れ、スケッチをアップロード
* ツール → シリアルモニタ で、シリアルモニタを開く
* "Do you update HSES-LMC1 firmware ?[y/n]" と聞かれるので、
シリアルモニタ上部の入力欄にyと入力し、「送信」をクリック

<center>
<img src="https://github.com/h-nari/Humblesoft_LedMat/blob/master/img/sc170831a2m.png?raw=true" /> </center>

* firmwareのダウンロード＆書込が行なわれる。
* 以下のような画面になれば、成功です。

<center>
<img src="https://github.com/h-nari/Humblesoft_LedMat/blob/master/img/sc170831a3m.png?raw=true" /> </center>

* うまく行かなかった場合、
<a href="http://www.humblesoft.com/bbs/c-board.cgi">サポート掲示板</a>で
問い合わせて下さい。


## firmwareの変更履歴

### ver 1.0.2

* 2017年8月30日リリース
* 表示モジュール数が多い場合(64x32dotモジュールで4枚以上) 、
表示ができなくなるバグを修正

### ver 0.0.39

* 2017年5月30日リリース

