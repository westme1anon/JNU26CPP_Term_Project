import io
import json
import os
import sys
from urllib import request, error

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8")
sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding="utf-8")


ELEMENT_MAP = {
    0: "中立",
    1: "火系",
    2: "水系",
    3: "草系",
    4: "机械系",
}


def log(message: str) -> None:
    print(message, file=sys.stderr, flush=True)


def has_advantage(attacker: int, defender: int) -> bool:
    return (attacker == 1 and defender == 3) or (attacker == 3 and defender == 2) or (attacker == 2 and defender == 1)


def fallback_strategy(round_num, player_first, player_action, player_hp, player_max_hp,
                      player_atk, player_element, enemy_hp, enemy_max_hp, enemy_atk,
                      enemy_element, available_cards, enemy_command_used, enemy_disarmed):
    cards = {c for c in available_cards.split(",") if c}
    has_attack = "1" in cards
    has_defense = "2" in cards
    has_heal = "3" in cards
    can_command = enemy_command_used == 0

    base_damage = max(1, enemy_atk)
    if has_advantage(enemy_element, player_element):
        base_damage *= 2

    my_hp_ratio = enemy_hp / enemy_max_hp if enemy_max_hp > 0 else 0
    enemy_hp_ratio = player_hp / player_max_hp if player_max_hp > 0 else 0
    heal_amount = max(1, int(enemy_atk * 0.8))
    hp_gap = enemy_max_hp - enemy_hp

    if enemy_disarmed == 1:
        if my_hp_ratio < 0.8 and (has_heal or can_command):
            return 3, "[本地AI] 当前处于缴械回合，优先回复保血。"
        if has_defense or can_command:
            return 2, "[本地AI] 当前处于缴械回合，改为防御等待下回合。"

    if player_first == 1 and player_action != 0:
        if player_action == 1 and (has_defense or can_command):
            return 2, "[本地AI] 后手观察到玩家攻击，选择防御反制。"
        if player_action == 2:
            if ((hp_gap >= heal_amount and has_heal) or (has_heal and my_hp_ratio < 0.9) or can_command):
                return 3, "[本地AI] 后手观察到玩家防御，选择回复安全过牌。"
            return 1, "[本地AI] 后手观察到玩家防御，但缺少合适回复，转为进攻。"
        if player_action == 3 and (has_attack or can_command):
            return 1, "[本地AI] 后手观察到玩家回复，选择攻击施压。"

    if my_hp_ratio < 0.25 and (has_heal or can_command):
        return 3, "[本地AI] 自身血量过低，优先回复。"
    if player_hp <= base_damage and (has_attack or can_command):
        return 1, "[本地AI] 发现斩杀线，直接攻击。"
    if has_advantage(enemy_element, player_element) and my_hp_ratio >= 0.4 and (has_attack or can_command):
        return 1, "[本地AI] 具备元素克制且血量健康，主动攻击。"
    if has_advantage(player_element, enemy_element) and my_hp_ratio < 0.5 and (has_defense or can_command):
        return 2, "[本地AI] 被元素克制且血量偏低，优先防御。"
    if 0.25 <= my_hp_ratio < 0.6 and hp_gap >= heal_amount and (has_heal or can_command):
        return 3, "[本地AI] 中等血量且回复收益可观，选择回复。"
    if enemy_hp_ratio < 0.4 and (has_attack or can_command):
        return 1, "[本地AI] 对手血量偏低，继续攻击压制。"
    if my_hp_ratio >= 0.6 and (has_attack or can_command):
        return 1, "[本地AI] 自身状态健康，优先攻击。"
    if has_attack or can_command:
        return 1, "[本地AI] 默认选择攻击。"
    if has_heal:
        return 3, "[本地AI] 缺少攻击手段，选择回复。"
    if has_defense:
        return 2, "[本地AI] 仅剩防御手段，选择防御。"
    return 1, "[本地AI] 无可用信息，保底返回攻击。"


def build_prompt(round_num, is_player_first, player_action, player_hp, player_max_hp, player_atk,
                 player_element, enemy_hp, enemy_max_hp, enemy_atk, enemy_element,
                 available_cards, enemy_command_used, enemy_disarmed) -> str:
    intent_info = ""
    if is_player_first == 1 and player_action != 0:
        actions_map = {1: "【攻击】", 2: "【防御】", 3: "【回复】"}
        intent_info = (
            f"\n作为后手，你观察到了对手本回合的意图是："
            f"{actions_map.get(player_action, '【未知】')}。你可以据此进行反制。"
        )
    else:
        intent_info = "\n你是本回合的【先手】，你需要率先做出决策，对手将根据你的决策应对。"

    card_map = {"1": "攻击牌", "2": "防御牌", "3": "回复牌"}
    avail_list = [card_map.get(card_id) for card_id in available_cards.split(",") if card_id in card_map]
    avail_str = "、".join(avail_list) if avail_list else "无"

    enemy_type = ELEMENT_MAP.get(enemy_element, "中立")
    player_type = ELEMENT_MAP.get(player_element, "中立")

    return f"""
【游戏规则系统】
这是一款回合制卡牌对战游戏。每回合双方各自从牌库抽 3 张手牌，你必须且只能从手牌中选择 1 张打出。

卡牌效果：
1. 攻击牌：造成基于属性体系计算的伤害。基础公式为 max(1, 攻击方ATK - 防守方DEF)。
   如果发生元素克制（火克草、草克水、水克火），伤害翻倍。
   如果目标处于易伤状态，每层额外承受 +50% 伤害。
   如果目标本回合处于防御姿态，伤害会被明显减免。
2. 防御牌：本回合大幅减伤。
   - 防反：若你防御时对手攻击，则对手【下回合】缴械，且永久失去一张攻击牌。
   - 防空惩罚：若你防御时对手没有攻击，则你获得 1 层易伤，且永久失去一张防御牌。
3. 回复牌：回复自身攻击力 × 80% 的生命值（向下取整），且不会超过最大生命值。

【指令机制】
双方各有一次“指令”机会，可无视手牌，强制选择攻击/防御/回复中的任意一种动作。
铁律：指令极其珍贵，不要为了空过而浪费。只有在斩杀、保命或完成关键反制时才值得使用。

【重要限制】
你作为 AI 没有背包系统，不能使用任何战斗外道具。你只能通过手牌出牌，或在必要时发动一次指令。

【当前战报】
第 {round_num} 回合
[你] {enemy_type} | ATK:{enemy_atk} | HP:{enemy_hp}/{enemy_max_hp}
[指令] {"【已用】只能从手牌中出牌" if enemy_command_used == 1 else "【可用】可无视手牌强行指定一种动作"}
[状态] {"【缴械】本回合无法攻击" if enemy_disarmed == 1 else "【正常】"}
[手牌] {avail_str}
[对手] {player_type} | ATK:{player_atk} | HP:{player_hp}/{player_max_hp}
{intent_info}

【决策规则】
- 如果指令已用，你只能从当前手牌中选择存在的牌。
- 如果指令未用，你可以正常出牌，也可以发动指令强行选择动作。
- 结合血量、克制关系、对手意图和手牌情况做决策。
- 请优先避免无意义的防空惩罚。

请先简要分析战况，最后在独立的一行仅输出一个数字：
1=攻击  2=防御  3=回复
"""


def extract_message_text(response_payload: dict) -> str:
    choices = response_payload.get("choices", [])
    if not choices:
        return ""

    first = choices[0]
    message = first.get("message", {})
    content = message.get("content", "")
    if isinstance(content, list):
        text_parts = []
        for item in content:
            if isinstance(item, dict) and item.get("type") == "text":
                text_parts.append(item.get("text", ""))
        return "".join(text_parts)
    if isinstance(content, str):
        return content
    return str(content)


def try_streaming_llm(prompt: str) -> int:
    api_url = os.getenv("BATTLE_AI_API_URL", "").strip()
    api_key = os.getenv("BATTLE_AI_API_KEY", "").strip()
    model = os.getenv("BATTLE_AI_MODEL", "").strip()
    if not api_url or not api_key or not model:
        raise RuntimeError("missing api config")

    body = json.dumps({
        "model": model,
        "messages": [
            {
                "role": "system",
                "content": "你是由大语言模型驱动的卡牌战斗 AI。请先简要分析战况，最后在独立的一行仅输出决策数字。"
            },
            {"role": "user", "content": prompt}
        ],
        "temperature": 0.2,
        "stream": True
    }).encode("utf-8")

    req = request.Request(
        api_url,
        data=body,
        headers={
            "Content-Type": "application/json",
            "Authorization": f"Bearer {api_key}",
        },
        method="POST",
    )

    accumulated = []
    printed_header = False

    with request.urlopen(req, timeout=30) as resp:
        log("[远程AI] 调用成功，开始流式输出思考过程。")

        for raw_line in resp:
            line = raw_line.decode("utf-8", errors="ignore").strip()
            if not line or not line.startswith("data: "):
                continue

            data_str = line[6:]
            if data_str == "[DONE]":
                break

            try:
                payload = json.loads(data_str)
            except json.JSONDecodeError:
                continue

            choices = payload.get("choices", [])
            if not choices:
                continue

            delta = choices[0].get("delta", {})
            chunk = None

            if "reasoning_content" in delta and delta["reasoning_content"]:
                chunk = delta["reasoning_content"]
            elif "content" in delta and delta["content"]:
                content = delta["content"]
                if isinstance(content, list):
                    piece_list = []
                    for item in content:
                        if isinstance(item, dict) and item.get("type") == "text":
                            piece_list.append(item.get("text", ""))
                    chunk = "".join(piece_list)
                else:
                    chunk = str(content)

            if not chunk:
                continue

            if not printed_header:
                log("[远程AI] 思考/分析：")
                printed_header = True

            print(chunk, file=sys.stderr, end="", flush=True)
            accumulated.append(chunk)

    if printed_header:
        print("", file=sys.stderr, flush=True)

    full_text = "".join(accumulated)
    for ch in reversed(full_text):
        if ch in {"1", "2", "3"}:
            return int(ch)
    raise RuntimeError("invalid streaming model output")


def try_non_streaming_llm(prompt: str) -> int:
    api_url = os.getenv("BATTLE_AI_API_URL", "").strip()
    api_key = os.getenv("BATTLE_AI_API_KEY", "").strip()
    model = os.getenv("BATTLE_AI_MODEL", "").strip()
    if not api_url or not api_key or not model:
        raise RuntimeError("missing api config")

    body = json.dumps({
        "model": model,
        "messages": [
            {
                "role": "system",
                "content": "你是由大语言模型驱动的卡牌战斗 AI。请先简要分析战况，最后在独立的一行仅输出决策数字。"
            },
            {"role": "user", "content": prompt}
        ],
        "temperature": 0.2
    }).encode("utf-8")

    req = request.Request(
        api_url,
        data=body,
        headers={
            "Content-Type": "application/json",
            "Authorization": f"Bearer {api_key}",
        },
        method="POST",
    )

    with request.urlopen(req, timeout=20) as resp:
        raw = resp.read().decode("utf-8", errors="ignore")

    payload = json.loads(raw)
    content = extract_message_text(payload)
    if not content:
        raise RuntimeError("empty model output")

    log("[远程AI] 调用成功。")
    log("[远程AI] 思考/分析：")
    log(content.strip())

    for ch in reversed(content):
        if ch in {"1", "2", "3"}:
            return int(ch)
    raise RuntimeError("invalid model output")


def call_remote_llm(prompt: str) -> int:
    try:
        return try_streaming_llm(prompt)
    except Exception as stream_exc:
        log(f"[远程AI] 流式调用失败，尝试非流式调用：{stream_exc}")
        return try_non_streaming_llm(prompt)


def main():
    if len(sys.argv) < 15:
        print("1")
        return

    round_num = int(sys.argv[1])
    player_first = int(sys.argv[2])
    player_action = int(sys.argv[3])
    player_hp = int(sys.argv[4])
    player_max_hp = int(sys.argv[5])
    player_atk = int(sys.argv[6])
    player_element = int(sys.argv[7])
    enemy_hp = int(sys.argv[8])
    enemy_max_hp = int(sys.argv[9])
    enemy_atk = int(sys.argv[10])
    enemy_element = int(sys.argv[11])
    available_cards = sys.argv[12]
    enemy_command_used = int(sys.argv[13])
    enemy_disarmed = int(sys.argv[14])

    prompt = build_prompt(
        round_num, player_first, player_action, player_hp, player_max_hp, player_atk,
        player_element, enemy_hp, enemy_max_hp, enemy_atk, enemy_element,
        available_cards, enemy_command_used, enemy_disarmed)

    try:
        action = call_remote_llm(prompt)
    except Exception as exc:
        log(f"[远程AI] 调用失败，切换本地策略：{exc}")
        action, reason = fallback_strategy(
            round_num, player_first, player_action, player_hp, player_max_hp,
            player_atk, player_element, enemy_hp, enemy_max_hp, enemy_atk,
            enemy_element, available_cards, enemy_command_used, enemy_disarmed)
        log(reason)

    print(str(action))


if __name__ == "__main__":
    try:
        main()
    except error.URLError as exc:
        log(f"[远程AI] 网络错误，切换本地保底输出：{exc}")
        print("1")
