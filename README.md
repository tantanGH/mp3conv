# MP3EX.X

MP3 to PCM converter for X680x0/Human68k

MP3ファイルを X68K(MSM6258V) ADPCM形式ファイル または 16bit符号付き big endian raw PCM形式ファイル(`*.s44`等) に変換します。

X680x0/Human68k 上で動作します。

注意：MP3のリアルタイム再生は将来やるかもしれませんが現状では対応していません。

---

### Install

MP3EXxxx.ZIP をダウンロードして展開し、MP3EX.X をパスの通ったディレクトリに置きます。

---

### How to use

引数をつけずに実行するか、`-h` オプションをつけて実行するとヘルプメッセージが表示されます。

    MP3EX.X - MP3 to PCM converter version 0.1.0 (2023/02/11) by tantan
    usage: mp3ex.x [options] <mp3-file> <pcm-file>
    options:
      -a ... output in ADPCM format (default)
      -p ... output in 16bit PCM format
      -u ... use 060turbo high memory
      -h ... show help message

`-a` または `-p` オプションで出力形式を選択します。デフォルトは X68k ADPCM (15.625kHz mono) 形式です。自動的にモノラル変換とダウンサンプリングを行います。

`-p` でPCM形式を選択した場合、16bit符号付き生PCMデータとなります。周波数およびステレオモノラルについては元のMP3に準じます。特に変換はされません。

注意：MP3ファイルは可変ビットレートのものにも対応していますが、MP3ファイル全体をメモリに読み込むようになっているため、メモリがとても沢山必要ですw

`-u` オプションを利用するとMP3読み込みバッファに060turbo.sysのハイメモリを使用します。060turbo/PhantomXで利用できるはずです。

出力された長時間 ADPCM 形式データについては、`adpcmplay.x` などで再生できます。

[ADPCMPLAY.X / ADPCMREC.X](https://www.vector.co.jp/soft/x68/art/se014381.html)

同様に長時間の S44 形式データについては、`pcm3pcm.x` などでまーきゅりーゆにっとから直接再生できます。

[PCM3PCM.X](https://www.vector.co.jp/soft/x68/art/se019752.html)

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

* 0.1.0 (2023/02/11) ... 初版