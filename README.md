# KZReversi
KZReversiのC#版。CPUのAI関連を全てDLL化した。

●終盤探索速度について(2020/03/23現在)<br>
<br>
有名な以下のベンチマークを使用しました。<br>
http://www.radagast.se/othello/ffotest.html<br>
<br>
※小文字は黒番、大文字は白番<br>
<br>
2020/03/23現在<br>
AMD(R) Ryzen 5 3600<br>
キャッシュサイズ32MB<br>

FFO#40 ( Exact:(a2:+38)   1.07sec node:  14.14[Mn] nps:13204[Knps] )<br>
FFO#41 ( Exact:(h4: +0)   1.74sec node:  23.52[Mn] nps:13501[Knps] )<br>
FFO#42 ( Exact:(g2: +6)   1.41sec node:  19.61[Mn] nps:13938[Knps] )<br>
FFO#43 ( Exact:(G3:-12)   4.27sec node:  55.22[Mn] nps:12922[Knps] )<br>
FFO#44 ( Exact:(B8:-14)   2.93sec node:  33.49[Mn] nps:11419[Knps] )<br>
<br>
FFO#45 ( Exact:(b2: +6)  18.65sec node: 270.28[Mn] nps:14490[Knps] )<br>
FFO#46 ( Exact:(b3: -8)   5.12sec node:  71.55[Mn] nps:13968[Knps] )<br>
FFO#47 ( Exact:(G2: +4)   2.26sec node:  35.23[Mn] nps:15588[Knps] )<br>
FFO#48 ( Exact:(F6:+28)  15.50sec node: 210.64[Mn] nps:13586[Knps] )<br>
FFO#49 ( Exact:(e1:+16)  26.35sec node: 362.19[Mn] nps:13745[Knps] )<br>
FFO#50 ( Exact:(d8:+10)  61.66sec node: 727.20[Mn] nps:11793[Knps] )<br>
<br>
FFO#51 ( Exact:(E2:+6)   35.82sec node: 537.78[Mn] nps:15014[Knps] )<br>
FFO#52 ( Exact:(a3:+0)   40.69sec node: 677.40[Mn] nps:16647[Knps] )<br>
FFO#53 ( Exact:(d8:-2)  262.56sec node:   4.41[Gn] nps:16805[Knps] )<br>
FFO#54 ( Exact:(c7:-2)  275.82sec node:   4.70[Gn] nps:17024[Knps] )<br>
FFO#55 ( Exact:(G6:+0) 1461.49sec node:  25.44[Gn] nps:17406[Knps] )<br>
<br>
FFO#56 ( Exact:(H5:+2)   110.97sec node:  1.89[Gn] nps:16998[Knps] )<br>
FFO#57 ( Exact:(a6:-10)  353.80sec node:  5.84[Gn] nps:16521[Knps] )<br>
FFO#58 ( Exact:(g1:+4)   340.83sec node:  6.86[Gn] nps:20134[Knps] )<br>
FFO#59 ( Exact:(g8:+64)    4.40sec node: 29.27[Mn] nps: 6556[Knps] )<br>
<br>
変更点：<br>
・64bitに移行、64bitOSの普及率を考えて32bitOSでは非対応にした<br>
・PV-LINE(最善手リスト)をステータスバーに通知する機能を実装<br>
　-->NPSが5%程度落ちた・・・<br>
・オーダリングの深さを見直し、終盤探索の速度を改善<br>

2016/06/09<br>
変更点：<br>
・ハッシュテーブルの構造を大幅変更<br>
・オーダリング処理のバグを修正(直すまでAIが激遅だった)<br>
・終盤探索インタフェース実装(事前探索～幅決め打ち終盤探索)<br>

FIX:<br>
終盤探索でまれに着手を返却しないことがある->ほぼ解決<br>
CPU同士の対戦時に中断ボタンを押すとアプリが死ぬ->解決<br>

2016/06/04<br>
バグ：<br>
終盤探索でまれに着手を返却しないことがある<br>
CPU同士の対戦時に中断ボタンを押すとアプリが死ぬ<br>

2016/06/01<br>
C#側の設計がおかしくて挙動が壊滅的・・・<br>
近いうちに再設計して一新する予定<br>
