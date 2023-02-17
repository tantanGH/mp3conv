# MP3EX.X

MP3 to ADPCM/PCM converter for X680x0/Human68k

MP3ファイルを X68K(MSM6258V) ADPCM形式ファイル または 16bit符号付き big endian raw PCM形式ファイル(`*.s44`等) に変換します。

X680x0/Human68k 上で動作します。

注意：低クロックの000実機だとめっちゃくちゃ時間がかかります。040turbo / 060turbo / PhantomX / エミュレータを推奨します。

---

### Install

MP3EXxxx.ZIP をダウンロードして展開し、MP3EX.X をパスの通ったディレクトリに置きます。

---

### How to use

引数をつけずに実行するか、`-h` オプションをつけて実行するとヘルプメッセージが表示されます。

    MP3EX.X - MP3 to PCM converter version 0.2.0 (2023/02/17) by tantan
    usage: mp3ex.x [options] <mp3-file> <pcm-file>
    options:
      -a ... output in ADPCM format (default)
      -p ... output in 16bit PCM format
      -u ... use 060turbo high memory
      -h ... show help message

`-a` または `-p` オプションで出力形式を選択します。デフォルトは X68k ADPCM (15.625kHz mono) 形式です。自動的にモノラル変換とダウンサンプリングを行います。

`-p` でPCM形式を選択した場合、16bit符号付き生PCMデータとなります。周波数およびステレオモノラルについては元のMP3に準じます。特に変換はされません。

注意：MP3ファイルは可変ビットレートのものにも対応していますが、MP3ファイル全体をメモリに読み込むようになっているため、メモリがとても沢山必要ですw

`-u` オプションを利用するとMP3読み込みバッファに060turbo.sysのハイメモリを使用します。060turbo または PhantomX 060モードで利用できるはずです。(060turboのみ実機とエミュレータで確認、PhantomXは未確認)

出力された長時間 ADPCM/PCM 形式データの内蔵ADPCMでの再生には拙作 `MP3EXP.X` が使えます。

[MP3EXP.X](https://github.com/tantanGH/mp3exp)

長時間 PCM 形式データについては、NOZさんの `pcm3pcm.x` などでま〜きゅり〜ゆにっとから直接再生できます。

[PCM3PCM.X](https://www.vector.co.jp/soft/x68/art/se019752.html)

M.Kamadaさんの `s44play.x` を使うとま〜きゅり〜ゆにっと無しでもOPMによるエミュレーションで PCM形式データを再生可能です(要マシンパワー)。

[S44PLAY.X / FMPPLAY.X](http://retropc.net/x68000/software/sound/stereopcm/s44play/)

---

### License

MP3デコードライブラリとして libmad 0.15.1b をx68k向けにコンパイルしたものを利用させて頂いており、ライセンスは libmad のもの (GPLv2) に準じます。

---

### Special Thanks

* xdev68k thanks to ファミべのよっしんさん
* HAS060.X on run68mac thanks to YuNKさん / M.Kamadaさん / GOROmanさん
* HLK301.X on run68mac thanks to SALTさん / GOROmanさん
* XEiJ & 060turbo.sys thanks to M.Kamadaさん

---

### History

* 0.2.0 (2023/02/17) ... document 更新とリファクタリング
* 0.1.1 (2023/02/12) ... メモリ確保失敗時にバスエラーが出ることがあったのを修正
* 0.1.0 (2023/02/11) ... 初版