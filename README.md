pmml-0.2p1-fork-201405
============================

このリポジトリについて
----------------------

MML(Music Macro Language)コンパイラのPMML(Practical Music Macro Language)をフォークしたリポジトリです。

PMMLは[会津大学](http://www.u-aizu.ac.jp/ "会津大学")の西村憲 先生が開発したMMLコンパイラです。MMLソースファイルからMIDIファイルを生成します。

オリジナルのPMMLは以下のURLからダウンロードできますが、pmml-0.2p1.tar.gz(1998/06/17)以降は新しいバージョンがリリースされていないようです。このリポジトリはpmml-0.2p1.tar.gzからフォークして、2014年時点でのUNIX系OSでPMMLがビルドできるようソースコードを修正しています。

 * [ftp://ftp.u-aizu.ac.jp/u-aizu/pmml/](ftp://ftp.u-aizu.ac.jp/u-aizu/pmml/ "ftp://ftp.u-aizu.ac.jp/u-aizu/pmml/")

以下のUNIX系OSでビルドが通ることを確認できています。Debianではgccの他にgpref(Perfect hash function generator)パッケージが必要です。

 * NetBSD-6.1-i386
 * Debian GNU/Linux 7.4 (wheezy)

ビルド方法
----------

GitHubからcloneしてmakeを走らせるだけです。configureは未対応です。インストール先をデフォルトの場所(/usr/local/bin,/usr/local/lib/pmml)から変更したい場合は、Makefile中のBINDIR,LIBDIRを修正してください。
