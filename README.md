# MP3CONV.X

MP3 to ADPCM/PCM converter for X680x0/Human68k

MP3ファイルを以下の形式のいずれかに変換します。

- X68K(MSM6258V) ADPCM形式ファイル(.PCM)
- 16bit符号付き big endian raw PCM形式ファイル(.S44等)
- 16bit符号付き YM2608 ADPCM形式ファイル(.A44等)

X680x0/Human68k 上で動作します。MP3ファイルは可変ビットレートのものにも対応しています。

注意：処理効率のため、MP3ファイルは一度にメモリに読み込みます。メモリフル実装またはハイメモリの利用を強く推奨します。

注意：MPUパワーを必要とするので、低クロックの000実機だとめちゃくちゃ時間がかかります。040turbo / 060turbo / PhantomX / エミュレータを推奨します。

このプログラムはプレーヤーではありません。MP3の再生には

[HMP3P.X](https://github.com/tantanGH/hmp3p)

の方を利用ください。

---

### Install

MP3CVxxx.ZIP をダウンロードして展開し、MP3CONV.X をパスの通ったディレクトリに置きます。

---

### How to use

引数をつけずに実行するか、`-h` オプションをつけて実行するとヘルプメッセージが表示されます。

    MP3CONV.X - MP3 to ADPCM/PCM converter for X680x0 version 0.x.x by tantan
    usage: mp3conv.x [options] <mp3-file>
    options:
      -a ... output in MSM6258V ADPCM format (default)
      -p ... output in 16bit Raw PCM format
      -n ... output in 16bit YM2608 ADPCM format
      -v<n> ... volume (1-192), default:100
      -h ... show help message
      -o <out-name> ... output filename (default:auto assign)

`-a` オプションで出力を X68k(MSM6258V) ADPCM (15.625kHz mono) 形式とします。デフォルトの出力形式です。自動的にモノラル変換とダウンサンプリングを行います。

`-p` オプションで出力を16bit符号付きraw PCMデータとします。`-a` と同時に指定はできません。周波数およびステレオモノラルについては元のMP3に準じます。特に変換はされません。

`-n` オプションで出力を16bit符号付きYM2608 ADPCM(ソフトウェアADPCM)データとします。`-a` `-p` と同時に指定はできません。周波数およびステレオモノラルについては元のMP3に準じます。

`-v` オプションで音量を調整します。

`-o`オプションの出力ファイル名を指定します。これを省略した場合、元のMP3ファイル名の拡張子を出力形式に合わせて自動的に変更したものとなります。
ファイルが既に存在する場合は確認を求められます。

- ADPCM出力 ... (元のMP3主ファイル名).pcm
- PCM出力でMP3が44.1kHzステレオの場合 ... (元のMP3主ファイル名).s44
- YM2608 ADPCM出力でMP3が44.1kHzステレオの場合 ... (元のMP3主ファイル名).a44

出力された長時間 MSM6258V-ADPCM/RawPCM/YM2608-ADPCM 形式データの内蔵ADPCMまたはMercury-Unitでの再生には拙作 `S44EXP.X` が使えます。

[S44EXP.X](https://github.com/tantanGH/s44exp)

M.Kamadaさんの `s44play.x` を使うとMercury-Unit無しでもOPMによるエミュレーションでステレオ16bitPCM形式データを再生可能です(要マシンパワー)。

[S44PLAY.X](http://retropc.net/x68000/software/sound/stereopcm/s44play/)

---

### License

MP3デコードライブラリとして libmad 0.15.1b をx68k向けにコンパイルしたものを利用させて頂いており、ライセンスは libmad のもの (GPLv2) に準じます。

YM2608 ADPCM形式のエンコードライブラリとして Otankonas氏のライブラリコードの一部を当方でデバッグしたものを利用させて頂いています。

---

### History

* 0.7.0 (2024/03/09) ... ハイメモリが利用可能な場合は自動的に利用するようにした
* 0.6.0 (2023/03/16) ... プログラム名称変更 -vオプション追加
* 0.5.0 (2023/02/28) ... N44への書き出しが正しくできていなかったのを修正
* 0.4.5 (2023/02/24) ... YM2608 ADPCMとしてA44/N44を正式サポートした
* 0.3.0 (2023/02/22) ... 全面的に書き直した
* 0.2.0 (2023/02/17) ... document 更新とリファクタリング
* 0.1.0 (2023/02/11) ... 初版
