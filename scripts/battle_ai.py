import io
import json
import os
import sys
from urllib import error, request

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8")
sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding="utf-8")


def log(message: str) -> None:
    print(message, file=sys.stderr, flush=True)


def parse_cards(card_text: str) -> set[str]:
    return {card for card in card_text.split(",") if card}


def format_cards(card_text: str) -> str:
    card_map = {"1": "攻击", "2": "防御", "3": "回复"}
    cards = [card_map[card_id] for card_id in card_text.split(",") if card_id in card_map]
    return "、".join(cards) if cards else "无"


def estimate_attack_damage(attacker_atk: int, defender_def: int, defender_vuln: int) -> int:
    base_damage = max(1, attacker_atk - defender_def)
    return max(1, int(base_damage * (1.0 + 0.5 * defender_vuln)))


def fallback_strategy(
    round_num,
    player_first,
    player_action,
    player_hp,
    player_max_hp,
    player_atk,
    player_def,
    player_vulnerability,
    player_cards_text,
    player_command_used,
    enemy_hp,
    enemy_max_hp,
    enemy_atk,
    enemy_def,
    enemy_vulnerability,
    enemy_cards_text,
    enemy_command_used,
    enemy_disarmed,
):
    del round_num
    del enemy_command_used

    player_cards = parse_cards(player_cards_text)
    enemy_cards = parse_cards(enemy_cards_text)
    player_can_command = player_command_used == 0

    player_has_attack = "1" in player_cards
    player_has_defense = "2" in player_cards
    player_has_heal = "3" in player_cards

    has_attack = "1" in enemy_cards
    has_defense = "2" in enemy_cards
    has_heal = "3" in enemy_cards

    my_damage = estimate_attack_damage(enemy_atk, player_def, player_vulnerability)
    player_damage = estimate_attack_damage(player_atk, enemy_def, enemy_vulnerability)
    my_hp_ratio = enemy_hp / enemy_max_hp if enemy_max_hp > 0 else 0.0
    player_hp_ratio = player_hp / player_max_hp if player_max_hp > 0 else 0.0
    heal_amount = max(1, int(enemy_atk * 0.8))
    hp_gap = enemy_max_hp - enemy_hp

    if enemy_disarmed == 1:
        if my_hp_ratio < 0.8 and has_heal:
            return 3, "[本地AI] 当前被缴械，优先回复保命。"
        if has_defense:
            return 2, "[本地AI] 当前被缴械，攻击无效，先防御过渡。"

    if player_first == 1 and player_action != 0:
        if player_action == 1 and has_defense:
            return 2, "[本地AI] 玩家已公开攻击意图，优先防御反制。"
        if player_action == 2:
            if (hp_gap >= heal_amount and has_heal) or (has_heal and my_hp_ratio < 0.9):
                return 3, "[本地AI] 玩家偏防守，趁机回复。"
            if has_attack:
                return 1, "[本地AI] 玩家偏防守，转为攻击施压。"
        if player_action == 3 and has_attack:
            return 1, "[本地AI] 玩家回复时立刻攻击施压。"

    if player_first == 0:
        if player_has_defense and not player_has_attack and not player_can_command and has_heal and my_hp_ratio < 0.9:
            return 3, "[本地AI] 玩家这回合偏防守，先回复争取收益。"
        if player_has_heal and not player_has_defense and not player_can_command and has_attack:
            return 1, "[本地AI] 玩家手里有回复空间，优先攻击施压。"
        if (player_has_attack or player_can_command) and has_defense and player_damage >= max(1, enemy_hp // 4):
            return 2, "[本地AI] 玩家具备明确进攻能力，提前防御更稳。"

    if player_hp <= my_damage and has_attack:
        return 1, "[本地AI] 存在斩杀机会，直接攻击。"
    if my_hp_ratio < 0.25 and has_heal:
        return 3, "[本地AI] 血量过低，优先回复。"
    if 0.25 <= my_hp_ratio < 0.6 and hp_gap >= heal_amount and has_heal:
        return 3, "[本地AI] 中等血量且回复收益较高，选择回复。"
    if player_hp_ratio < 0.4 and has_attack:
        return 1, "[本地AI] 玩家血量偏低，继续攻击压制。"
    if my_hp_ratio >= 0.6 and has_attack:
        return 1, "[本地AI] 当前状态健康，优先攻击。"
    if has_attack:
        return 1, "[本地AI] 默认选择攻击。"
    if has_heal:
        return 3, "[本地AI] 缺少攻击牌，选择回复。"
    if has_defense:
        return 2, "[本地AI] 仅剩防御牌，选择防御。"
    return 1, "[本地AI] 没有有效手牌信息，保底攻击。"


def build_prompt(
    round_num,
    player_first,
    player_action,
    player_hp,
    player_max_hp,
    player_atk,
    player_def,
    player_vulnerability,
    player_cards_text,
    player_command_used,
    enemy_hp,
    enemy_max_hp,
    enemy_atk,
    enemy_def,
    enemy_vulnerability,
    enemy_cards_text,
    enemy_command_used,
    enemy_disarmed,
) -> str:
    actions_map = {1: "攻击", 2: "防御", 3: "回复"}
    enemy_damage = estimate_attack_damage(enemy_atk, player_def, player_vulnerability)
    player_damage = estimate_attack_damage(player_atk, enemy_def, enemy_vulnerability)
    enemy_status = "本回合无法攻击" if enemy_disarmed == 1 else "正常"

    if player_first == 1 and player_action != 0:
        intent_info = (
            f"你是后手，已经看到玩家本回合公开动作：{actions_map.get(player_action, '未知')}。\n"
            "你应直接利用这个已知信息反制。"
        )
    else:
        intent_info = (
            "你是先手，看不到玩家最终动作，但你已经知道玩家本回合抽到的手牌类型。\n"
            "请根据这些已知手牌推断玩家更可能的动作。"
        )

    return f"""
你是校园RPG的敌方战斗AI。

硬性规则：
1. 你只能从自己当前手牌中选择一个真实存在的动作。
2. 你绝对不能使用“指令”。
3. 你绝对不能使用背包或战斗道具。
4. 你不能把下列明示数值说成“未知”；这些数值就是你当前已经知道的战斗信息。

指令说明：
- 指令是玩家每场战斗只有一次的强制行动机会。
- 玩家若指令=可用，则可以无视当前手牌，强制执行攻击、防御、回复，或使用一次战斗道具。
- 玩家若指令=已用，则本回合只能从当前手牌里出牌。
- 你自己没有指令可用，永远只能从自己的当前手牌出牌。

战斗规则摘要：
- 攻击基础伤害 = max(1, 攻击方ATK - 防守方DEF)
- 易伤：目标每层额外承受 +50% 伤害
- 防御：能减伤；若成功对上攻击，可使对手下回合缴械
- 回复：恢复自身 ATK 的 50% 生命

当前战况：
- 回合数：{round_num}
- 你：HP={enemy_hp}/{enemy_max_hp}，ATK={enemy_atk}，DEF={enemy_def}，易伤={enemy_vulnerability}，状态={enemy_status}，指令=不可用
- 你的手牌：{format_cards(enemy_cards_text)}
- 玩家：HP={player_hp}/{player_max_hp}，ATK={player_atk}，DEF={player_def}，易伤={player_vulnerability}，指令={'已用' if player_command_used == 1 else '可用'}
- 玩家本回合手牌：{format_cards(player_cards_text)}
- 若你本回合攻击玩家，预计伤害约为：{enemy_damage}
- 若玩家本回合攻击你，预计伤害约为：{player_damage}

局面补充：
{intent_info}

决策要求：
- 先做简短分析，但不要编造未知属性
- 如果玩家指令仍可用，不要因为玩家当前没抽到某张牌，就断定他绝对做不出对应动作
- 优先避免明显吃亏的防空或无意义回复
- 只能在最后单独输出一个数字

输出规则：
1=攻击
2=防御
3=回复
"""


def extract_message_text(response_payload: dict) -> str:
    choices = response_payload.get("choices", [])
    if not choices:
        return ""

    message = choices[0].get("message", {})
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

    body = json.dumps(
        {
            "model": model,
            "messages": [
                {
                    "role": "system",
                    "content": "你是校园RPG的敌方战斗AI。你不能使用指令，只能从当前手牌中选择攻击、防御、回复之一。请基于输入里明确给出的属性与手牌做判断，不要把已给出的ATK、DEF、HP、易伤说成未知。最后单独输出数字决策。",
                },
                {"role": "user", "content": prompt},
            ],
            "temperature": 0.2,
            "stream": True,
        }
    ).encode("utf-8")

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
        log("[远程AI] 调用成功，开始流式输出分析。")

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
                log("[远程AI] 分析：")
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

    body = json.dumps(
        {
            "model": model,
            "messages": [
                {
                    "role": "system",
                    "content": "你是校园RPG的敌方战斗AI。你不能使用指令，只能从当前手牌中选择攻击、防御、回复之一。请基于输入里明确给出的属性与手牌做判断，不要把已给出的ATK、DEF、HP、易伤说成未知。最后单独输出数字决策。",
                },
                {"role": "user", "content": prompt},
            ],
            "temperature": 0.2,
        }
    ).encode("utf-8")

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
    log("[远程AI] 分析：")
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
    if len(sys.argv) < 19:
        print("1")
        return

    round_num = int(sys.argv[1])
    player_first = int(sys.argv[2])
    player_action = int(sys.argv[3])
    player_hp = int(sys.argv[4])
    player_max_hp = int(sys.argv[5])
    player_atk = int(sys.argv[6])
    player_def = int(sys.argv[7])
    player_vulnerability = int(sys.argv[8])
    player_cards_text = sys.argv[9]
    player_command_used = int(sys.argv[10])
    enemy_hp = int(sys.argv[11])
    enemy_max_hp = int(sys.argv[12])
    enemy_atk = int(sys.argv[13])
    enemy_def = int(sys.argv[14])
    enemy_vulnerability = int(sys.argv[15])
    enemy_cards_text = sys.argv[16]
    enemy_command_used = int(sys.argv[17])
    enemy_disarmed = int(sys.argv[18])

    prompt = build_prompt(
        round_num,
        player_first,
        player_action,
        player_hp,
        player_max_hp,
        player_atk,
        player_def,
        player_vulnerability,
        player_cards_text,
        player_command_used,
        enemy_hp,
        enemy_max_hp,
        enemy_atk,
        enemy_def,
        enemy_vulnerability,
        enemy_cards_text,
        enemy_command_used,
        enemy_disarmed,
    )

    try:
        action = call_remote_llm(prompt)
    except Exception as exc:
        log(f"[远程AI] 调用失败，切换本地策略：{exc}")
        action, reason = fallback_strategy(
            round_num,
            player_first,
            player_action,
            player_hp,
            player_max_hp,
            player_atk,
            player_def,
            player_vulnerability,
            player_cards_text,
            player_command_used,
            enemy_hp,
            enemy_max_hp,
            enemy_atk,
            enemy_def,
            enemy_vulnerability,
            enemy_cards_text,
            enemy_command_used,
            enemy_disarmed,
        )
        log(reason)

    print(str(action))


if __name__ == "__main__":
    try:
        main()
    except error.URLError as exc:
        log(f"[远程AI] 网络错误，切换保底输出：{exc}")
        print("1")
