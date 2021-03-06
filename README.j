このパッケージはPMML関連ソフトウェアのリリース 0.2 であり、
PMML コンパイラ（これまでインタプリタと呼ばれていたもの）、
逆コンパイラ、マクロライブラリ、Emacs インターフェース、
及びいくつかの音楽作品例を含んでいます。PMML コンパイラは、
PMML (Practical Music Macro Language) と呼ばれる音楽記述言語を
標準MIDIファイルへ変換します。また、PMMLコンパイラは標準MIDI
ファイルを操作するための汎用ツールとしても使用できます。
例えば、いくつかのMIDIファイルを併合したり、MIDIファイル中の音符
を移調したり、MIDIファイルを任意のフォーマットのテキストに変換する
ことが可能です。逆コンパイラは、標準MIDIファイルをPMMLへ変換します。

PMMLとは？
----------

PMML(Practical Music Macro Language)はコンピュータによるMIDI楽器の
自動演奏を目的とした音楽記述言語です。
直接記述、アルゴリズム作曲、音楽の変形をすべてサポートしています。
主な特徴は以下の通りです。

・ C や D# といった音名を並べることによってフレーズを直接的に表現すること
   が可能です。和音や並行声部も分かりやすい形で表現できます。さらに、
   アクセント、時刻微調整、クレッシェンド、テンポルバートなどの表情づけ
   を簡単に記述することができます。

・ エクスクルーシブを含む標準MIDIファイルにおけるすべての種類のイベント
   をサポートしています。

・ 連続的に変化するコントロールチェンジの列を生成することがきます。
   変化の形状としては、直線の他に自由曲線を利用したものがあります。

・ Cライクな条件分岐やループなどの制御構文を持っています。

・ 強力なマクロ機能を備えています。
   マクロは共通するフレーズや伴奏パターンを定義するために特に有用です。

・ エフェクタと呼ばれるイベント処理モジュールが用意されています。
   エフェクタは、記述された音楽の各イベントに対してユーザ定義可能な
   アクションを適用します。これによって、ピッチ変換、 時間軸変換、和音化、
   和音のアルペジオ化等の音楽の変形を実現できます。

・ 外部MIDIファイルの内容を記述された音楽に挿入することができます。
   エフェクタは外部MIDIファイルのイベントに対しても適用できます。

Release 0.1 との違い
--------------------

・ 逆コンパイラ(m2p)が新たに加わりました。出力形式としては、
   １行１イベントを原則とする形式と１行１小節を原則とする
   形式の２通りがあります。連続コントロールチェンジは可能な限り直線へ
   の当てはめが行われます。１行１小節の形式では、和音が角括弧で
   囲んで表示されます。逆コンパイラのマニュアルはまだありませんが、
   m2p -h によって使用法の概要を知ることができます。

・ PMMLインタプリタはPMMLコンパイラへと名称を変更しました。

・ Texinfo形式のマニュアルが一緒に配布されるようになりました。

・ PMMLコンパイラにおいて、有理数から整数／浮動小数点数への型変換、
   整数／浮動小数点数から有理数への型変換規則が変わり、ティック数を
   基準にして変換されるようになりました。 

・ その他の細かい相違については、各ディレクトリの ChangeLog を御覧下さい。

パッケージの内容
----------------

   COPYING    - GNU 一般公有使用許諾書
   COPYING.j  - GNU 一般公有使用許諾書の日本語訳
   INSTALL    - インストールの方法（英語）
   INSTALL.j  - インストールの方法（日本語, EUCコード）
   Makefile   - トップレベルの Makefile
   README     - このファイルの英語版
   README.j   - このファイル
   c_lib      - 筆者によって書かれたものではないが、便宜上一緒に配布
                されているＣプログラム
   common     - 共通ルーチンのＣソースコード
   comp       - PMMLコンパイラのＣソースコード
   emacs      - PMML編集モードのための Emacs Lisp プログラム
   examples   - PMMLで書かれた音楽作品例。
                グリーグのピアノ協奏曲第１楽章を含む。
   lib        - PMMLマクロ／エフェクタライブラリのファイル
   m2p        - 逆コンパイラのＣソースコード
   manual     - Texinfo形式マニュアル

移植性
------

PMMLコンパイラ／逆コンパイラは移植性の高いソフトウェアです。端末I/O、
ウィンドウI/O、MIDI I/O、及び非標準のライブラリは使用していません。
唯一の本質的な制約は、IEEE浮動小数点フォーマットを前提していることだと
思われます。現在までに、次のOSとＣコンパイラの組合せで動作を確認しています。

SunOS 4.1.3 - cc and gcc 2.7.2
SunOS 5.5.1 - cc and gcc 2.7.2
Linux Slackware 3.0 - gcc 2.7.0
IRIX 5.2 -- cc and gcc 2.7.2
IRIX 6.2 -- cc (with the -cckr option)
IRIX 6.2 -- gcc 2.7.2
DOS/V 6.3 -- gcc 2.6.3(djgpp)

複製、配布、改変について
------------------------

このパッケージ（ただし c_lib ディレクトリにあるファイルを除く）は
GNUプロジェクトの一環として開発されたものではありませんが、
複製、配布、改変の条件についてはGNU 一般公有使用許諾書に従います。

注意事項
--------

PMMLコンパイラはまだ開発途上段階にあります。このため、PMMLの
言語仕様は予告なく変更されることがあります。また、新しいバージョンの
PMMLコンパイラがリリースされたときに、古いバージョンとの互換性は
必ずしも保証されません。

本パッケージは完全に無保証です。詳細についてはGNU 一般公有使用許諾書を
お読みください。

マニュアル
----------

日本語版のPMMLユーザーズマニュアルは manual ディレクトリの中にあります。

バグレポート
------------

バグレポート、御意見、御感想を nisim@u-aizu.ac.jp までお寄せ下さい。

ホームページURL （バージョンアップ情報など）
-------------------------------------------

http://www-cgl.u-aizu.ac.jp/pmml/index-j.html

著作者
------

会津大学コンピュータ理工学部コンピュータソフトウェア学科
西村 憲
Email: nisim@u-aizu.ac.jp
