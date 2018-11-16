# KZReversi
KZReversiのC#版。CPUのAI関連を全てDLL化した。

●終盤探索速度について(2018/11/16現在)<br>
<br>
有名な以下のベンチマークを使用しました。<br>
http://www.radagast.se/othello/ffotest.html<br>
<br>
環境:CPU Intel(R) Core(TM) i5-6500 CPU @ 3.20GHz<br>
<br>
FFO#40 ( Exact:(a2:+38)   1.29sec node:  10.63[Mn] nps: 8244[Knps] )<br>
FFO#41 ( Exact:(h4: +0)   2.97sec node:  25.54[Mn] nps: 8599[Knps] )<br>
FFO#42 ( Exact:(g2: +6)   2.24sec node:  20.58[Mn] nps: 9189[Knps] )<br>
FFO#43 ( Exact:(C7:-12)   2.54sec node:  19.23[Mn] nps: 7572[Knps] )<br>
FFO#44 ( Exact:(B8:-14)   4.32sec node:  32.07[Mn] nps: 7418[Knps] )<br>
<br>
FFO#45 ( Exact:(b2: +6)  27.68sec node: 294.61[Mn] nps:10644[Knps] )<br>
FFO#46 ( Exact:(b3: -8)   7.56sec node:  68.56[Mn] nps: 9070[Knps] )<br>
FFO#47 ( Exact:(G2: +4)   3.25sec node:  36.70[Mn] nps:11293[Knps] )<br>
FFO#48 ( Exact:(F6:+28)  21.11sec node: 195.99[Mn] nps: 9286[Knps] )<br>
FFO#49 ( Exact:(e1:+16)  34.84sec node: 346.90[Mn] nps: 9958[Knps] )<br>
FFO#50 ( Exact:(d8:+10) 108.94sec node: 960.91[Mn] nps: 8820[Knps] )<br>
<br>
FFO#51 ( Exact:(E2:+6)   36.21sec node: 378.54[Mn] nps:10453[Knps] )<br>
FFO#52 ( Exact:(a3:+0)   63.95sec node: 730.82[Mn] nps:11429[Knps] )<br>
FFO#53 ( Exact:(d8:-2)  545.77sec node:   6.17[Gn] nps:11304[Knps] )<br>
FFO#54 ( Exact:(c7:-2)  626.09sec node:   7.42[Gn] nps:11848[Knps] )<br>
FFO#55 ( Exact:(G6:+0) 2492.74sec node:  31.10[Gn] nps:12475[Knps] )<br>
<br>
FFO#56 ( Exact:(H5:+2)   212.26sec node:  2.52[Gn] nps:11894[Knps] )<br>
FFO#57 ( Exact:(a6:-10)  520.85sec node:  6.35[Gn] nps:12183[Knps] )<br>
FFO#58 ( Exact:(g1:+4)   588.80sec node:  8.54[Gn] nps:14512[Knps] )<br>
FFO#59 ( Exact:(g8:+64)    1.88sec node:  8.86[Mn] nps: 4722[Knps] )<br>
<br>
※小文字は黒番、大文字は白番<br>
<br>
2016/08/14<br>
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
