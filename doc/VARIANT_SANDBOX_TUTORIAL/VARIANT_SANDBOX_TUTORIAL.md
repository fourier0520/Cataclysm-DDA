
# Variant_JP_sandbox ベータ版 チュートリアル

## Sandboxとは？

Sandboxは、簡単に言えば、Webブラウザからだけでもヴァリアントの新機能を作れるシステムです！

https://github.com/CleverRaven/Cataclysm-DDA/blob/master/doc/COMPILING/COMPILING.md

本来はこれ↑の通りに環境を作らないとCDDAの開発はできませんが、

Sandboxを利用すればちょっとばかりのプログラミングの知識があればすぐに作り始められます！

## ベータ版？

Sandboxシステムが時々止まったり、上手く動かなかったりする可能性があります。

Discordなどでお知らせして頂ければ、出来る限りですが対応します。

https://discord.gg/kGNU8Jg

https://ux.getuploader.com/cdda_variant_jp_sandbox/

## チュートリアル

### わかってる人向けに一言で言うと

このリポジトリのvariant_jp_sandboxブランチ宛にプルリクエストを送ってみてください。

すぐに勝手に承認されて、しばらく待つと、プルリクエストのコメントに、エラーならビルドエラー、成功ならアップロード先へのURLが書き込まれます。

### 必要なコンピュータスキルについて

- CDDAの本体はC++で出来ています。C++そのものや、あるいはC言語やJava、Javascript等の言語を触ったことがある人なら、多少の改造なら問題なくできると思います。
- CDDAのソースコードを読んで、なんとなくでも仕様を調べたりしたことがある人でも、十分やれると思います。
- CDDAのmodを作ったり、CDDAのJSONファイルを検索して、アイテムやレシピ等のデータを調べたりしたことがある人なら、もしかしたら分かると思います。
- gitについては知らなくても大丈夫なように書きました。

## チュートリアル

### アカウントの準備

- まずは、持っていなければ、Githubのアカウントを作ってください。
- これを見ているなら、ページの一番右上に、「Sign up」というボタンがあると思うので、そこから登録できます。
- Githubのアカウントをお持ちであれば、サインインしてください。

### フォーク

- Githubのヴァリアントのページを開いてください。
- https://github.com/roloa/Cataclysm-DDA_variant
- 右上に「Fork」というボタンがあるので、これを押します。

![図1](https://raw.githubusercontent.com/roloa/Cataclysm-DDA_variant/variant_jp/doc/VARIANT_SANDBOX_TUTORIAL/1.jpg)

### variant_jp_sandboxを開く

- すると、しばらくののち、次のような画面になります。
- 左上のタイトルに、自分の名前が入っていることを確認してください。
- そうしたら、図の赤丸で示したプルダウンメニューを開いて、
- 「variant_jp_sandbox」という項目を探してクリックしてください。

![図2](https://raw.githubusercontent.com/roloa/Cataclysm-DDA_variant/variant_jp/doc/VARIANT_SANDBOX_TUTORIAL/2.jpg)

### ソースファイルを開く

- プルダウンを選ぶと同じような画面に切り替わりますが、ちゃんと「variant_jp_sandbox」を選べているかをチェックしてください。
- 問題なければ
- Cataclysmの本体を改造するには、「src」というフォルダの中身を編集します。
- ファイル一覧から「src」フォルダを探してクリックしてください。


![図3](https://raw.githubusercontent.com/roloa/Cataclysm-DDA_variant/variant_jp/doc/VARIANT_SANDBOX_TUTORIAL/3.jpg)

- srcフォルダの中にはたくさんのプログラムの元となるソースファイルが入っています。
- どのファイルに何の機能が書かれているかは、今ここでは細かくは説明できませんが、
- 例として、新規キャラクター作成機能の書いてある「newcharacter.cpp」を少し改造してみます。

![図4](https://raw.githubusercontent.com/roloa/Cataclysm-DDA_variant/variant_jp/doc/VARIANT_SANDBOX_TUTORIAL/4.jpg)

- ファイルをクリックすると、ファイルの内容を表示する画面になります。
- 下図の赤丸の、鉛筆ボタンをクリックすると、編集画面に移動できます。

### ソースファイルを編集する

![図5](https://raw.githubusercontent.com/roloa/Cataclysm-DDA_variant/variant_jp/doc/VARIANT_SANDBOX_TUTORIAL/5.jpg)

- ブラウザ上の編集画面はちょっと使いにくいですが、ブラウザの手軽さと引き換えにがまんしましょう...
- 今回は例として、キャラクター作成時のステータス（筋肉、器用、知性、感覚）の最低値を、4よりも低くしてみます。
- 下図で示しているのは、ステータス設定画面で、左キーを押したときにステータスを減らす処理の仕組みが書いてあるところのようです。
- 左キーが押された時に、選択（selが1なら筋肉、2なら器用...）されたステータス（u.str_maxとか）が、4より大きい場合は、
  ステータスがHIGH_STAT(おそらくステ上げに2ポイント必要になる境の12のこと)よりも大きければ、特質ポイントをさらに1増やして、
  それから特質ポイントを1増やして、ステータスを1減らすというような処理のようです。
- ファイルの編集が終わったら、ページ下部のCommit changesの欄に、編集内容の概要を書いて（書かないでも問題ないけど）
  2つ並んだラジオボタンはそのまま上を選択したままにして、緑色のボタンを押してファイルの編集を確定させます。

![図6](https://raw.githubusercontent.com/roloa/Cataclysm-DDA_variant/variant_jp/doc/VARIANT_SANDBOX_TUTORIAL/6.jpg)

- ファイルの編集が終わると、ファイルの内容表示の画面に戻ってくるので、
- 下図の赤丸で示したリンクをクリックしてください。
- ページ上にも似たようなリンクがありますが、これをクリックすると手順が少し増えてやっかいなので気を付けましょう。

![図7](https://raw.githubusercontent.com/roloa/Cataclysm-DDA_variant/variant_jp/doc/VARIANT_SANDBOX_TUTORIAL/7.jpg)

### プルリクエスト(PR, Pull Request)を送る

- トップページに戻ってくると、下図のような黄色い背景と緑色のボタンが出てくると思います。
- 赤丸で囲んだ方の「Compare & pull request」の緑のボタンを押せば、プルリクエスト作成画面を開くことができます。
- もし、このボタンが見つからなかったりしたら、左の矢印で示した、白い「Pull request」のボタンも同じ機能を持ちます。
- その場合は、プルダウンメニューが「variant_jp_sandbox」を選んでいるかどうかを今一度確認してみてください。

![図8](https://raw.githubusercontent.com/roloa/Cataclysm-DDA_variant/variant_jp/doc/VARIANT_SANDBOX_TUTORIAL/8.jpg)

- プルリクエスト作成画面では少し確認することが多いです。
- まず、下図はちょっと見にくいですが、もし緑のチェックマークではなく赤いバツ等が表示された場合は、プルリクエストを送る前に、Discordなどでご相談ください。
- 上側の赤線で示した、「送り先のブランチ」の、「roloa/Cataclysm-DDA_variant」「variant_jp_sandbox」を確認します。
- 下側の赤線で示した、「送り元のブランチ」の、「(自分の名前)/Cataclysm-DDA_variant」「variant_jp_sandbox」を確認します。
- 「送り先のブランチ」には特に注意してください。
   送り先の選択肢の中には他のユーザーや、怖いことに「CleverRaven/Cataclysm-DDA」（本家ですよ！）もあるので、間違えて送ってしまうと、向こうの方をビックリさせてしまうかもしれません。
- コメントの内容は、デフォルトで英語で色々書いてあると思いますが、そのままや空白でも大丈夫です。
- 確認が出来たら、出来れば再度、送り先のブランチを確認してから、緑色の「Create PullRequest」ボタンをクリックしてください。

![図9](https://raw.githubusercontent.com/roloa/Cataclysm-DDA_variant/variant_jp/doc/VARIANT_SANDBOX_TUTORIAL/9.jpg)

### 結果を待つ

- プルリクエストを無事に送ることができると、下図のような画面になります。
- ここで、赤いバツが出ることがありますが、これはJSONの改行の場所が美しくないなどの些細な問題なので、気にしなくても大丈夫です。
- このまま待つと、正常であれば1分以内に、紫色の「Merged」という表示に自動的に変わります。
- 1分経っても変わらない場合は、Sandboxシステムが停止しているか、別のリクエストを処理している最中の可能性があります。
- 紫色のMergedが出てから、改造の内容にもよりますが、5分程度で結果が自動的に表示されます。
- 仮にすべてのファイルを改造したりした場合は、確実に1時間以上かかります。
- 表示される結果は、プログラムの間違っている場所を示した英語のエラー文か、アップローダーのURLを含んだ日本語のメッセージのどちらかです。

![図10](https://raw.githubusercontent.com/roloa/Cataclysm-DDA_variant/variant_jp/doc/VARIANT_SANDBOX_TUTORIAL/10.jpg)

- ちなみに、もしプルリクエストの画面を閉じてしまった場合は、プルリクエストのリストの「Closed」をチェックしてみてください。
- プルリクエストのリストは、画面の上に並んでいるタブから選べば開けます。

![図11](https://raw.githubusercontent.com/roloa/Cataclysm-DDA_variant/variant_jp/doc/VARIANT_SANDBOX_TUTORIAL/11.jpg)

- 無事に改造が成功した場合は、送られてくるアップローダーのURL先に成果がアップロードされているはずです。
- ゲーム本体のexeファイルのみの「(日付時刻)\_exe」と、ゲーム起動に必要なファイル全てが入った「(日付時刻)\_full.zip」の2つがアップロードされますので、必要な方を選んでください。
- Sandboxで作られるexeファイルは、JPヴァリアント仕様なので、JPヴァリアントのexeファイルと同じ場所に置けば動かせるはずです。
- よくわからない人は、「full」の方をダウンロードして使ってください。
- ちなみにシステムは「exe」がアップロード完了した時点で成功の報告をして「full」はその後でアップロード準備を始めるので、5分ほど時間差があります。
焦らずお待ちください。
- アップローダーのURLは以下です。
- https://ux.getuploader.com/cdda_variant_jp_sandbox/

![図12](https://raw.githubusercontent.com/roloa/Cataclysm-DDA_variant/variant_jp/doc/VARIANT_SANDBOX_TUTORIAL/12.jpg)

### 動作確認をする

- 思う存分動作確認をして、おもしろい機能が作れたら、皆さんにシェアしましょう！

![図13](https://raw.githubusercontent.com/roloa/Cataclysm-DDA_variant/variant_jp/doc/VARIANT_SANDBOX_TUTORIAL/13.jpg)


