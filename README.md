# KZReversi
KZReversiのC#版。CPUのAI関連を全てDLL化した。

●終盤探索速度について(2016/8/14現在)<br>
<br>
有名な以下のベンチマークを使用しました。<br>
http://www.radagast.se/othello/ffotest.html<br>
<br>
環境:CPU Intel(R) Core(TM) i5-6500 CPU @ 3.20GHz<br>
<br>
FFO#40 ( Exact:(a2:+38) 1.04sec node: 10.26[Mn] )<br>
FFO#41 ( Exact:(h4: +0) 3.22sec node: 37.93[Mn] )<br>
FFO#42 ( Exact:(g2: +6) 4.01sec node: 55.72[Mn] )<br>
FFO#43 ( Exact:(G3:-12) 13.10sec node: 125.5[Mn] )<br>
FFO#44 ( Exact:(D2:-14) 3.22sec node: 30.26[Mn] )<br>
<br>
FFO#45 ( Exact:(b2: +6)  58.63sec node: 789.30[Mn] )<br>
FFO#46 ( Exact:(b3: -8)  10.27sec node: 123.50[Mn] )<br>
FFO#47 ( Exact:(G2: +4)   4.60sec node:  58.96[Mn] )<br>
FFO#48 ( Exact:(F6:+28)  36.09sec node: 426.40[Mn] )<br>
FFO#49 ( Exact:(e1:+16)  50.33sec node: 584.82[Mn] )<br>
FFO#50 ( Exact:(d8:+10) 354.14sec node:   2.91[Gn] )<br>
<br>
FFO#51 ( Exact:(E2:+6)   59.20sec node: 732.80[Mn] )<br>
FFO#52 ( Exact:(a3:+0)  142.79sec node:   3.15[Gn] )<br>
FFO#53 ( Exact:(d8:-2)  656.87sec node:  19.16[Gn] )<br>
FFO#54 ( Exact:(c7:-2) 1718.65sec node:  24.51[Gn] )<br>
FFO#55 ( Exact:(G6:+0) 5588.48sec node:  80.07[Gn] )<br>
<br>
FFO#56 ( Exact:(H5:+2)   314.27sec node:   3.72[Gn] )<br>
FFO#57 ( Exact:(a6:-10) 1045.01sec node:  12.54[Gn] )<br>
FFO#58 ( Exact:(g1:+4)   973.58sec node:  12.65[Gn] )<br><br>
FFO#59 ( Exact:(g8:+64)    0.25sec node: 938.18[Kn] )<br>
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
