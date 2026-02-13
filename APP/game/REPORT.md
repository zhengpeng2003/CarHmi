# Qt 斗地主问题排查报告

## 1. “要不起”标签未显示
- 事件信号未触发：`gamecontrol::GamePlayhand` 在收到空牌（要不起）时直接返回，未向 UI 层发送 `S_gamePlayHand`，导致 `Maingame::OndisPosePlayhand` 中的“要不起”标签和音效逻辑完全没有被调用。【F:gamecontrol.cpp†L166-L210】【F:maingame.cpp†L481-L518】
- UI 显示逻辑存在但得不到触发：`OndisPosePlayhand` 中已将空牌视为“要不起”，会设置 `pass.png` 并显示 `_NOCardlabel`，但由于上层信号未发出，界面不会更新。【F:maingame.cpp†L481-L518】

## 2. “要不起”无声音
- 与标签问题同源：音效播放在 `OndisPosePlayhand` 的“要不起”分支内调用 `_Bgmcontrol->NoPlayerHandBgm(...)`，同样依赖 `S_gamePlayHand` 信号。信号缺失导致音效代码不运行。【F:maingame.cpp†L481-L518】
- 配置存在：`conf/playList.json` 中包含 `Man_buyao1-4`、`Woman_buyao1-4` 资源，说明资源不是缺失问题。【F:conf/playList.json†L1-L93】

## 3. 结算后“继续游戏”卡死
- 重新开局时直接对 `_Playercontexts` 的 `_Last_Cards` 解引用清空，若上一局在 `HidePlayhand` 中将该指针置为 `nullptr`，此处会空指针解引用，造成崩溃/无响应。【F:maingame.cpp†L520-L540】【F:maingame.cpp†L703-L736】
- 游戏状态清理流程中其他重置调用（`RetCardDate`、`ClearScore` 等）都在执行，说明主要风险来自对 `_Last_Cards` 的不安全访问。【F:maingame.cpp†L703-L733】【F:gamecontrol.cpp†L110-L135】

