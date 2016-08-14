# KZReversi
KZReversiのC#版。CPUのAI関連を全てDLL化した。

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
