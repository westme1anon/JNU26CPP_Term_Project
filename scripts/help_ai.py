import json
import os
import sys
import urllib.error
import urllib.request


SYSTEM_PROMPT = (
    "你是校园 RPG 游戏 CampusRPG 的 AI 助手。"
    "请根据玩家当前状态，给出一条简短、直接、可执行的下一步建议。"
)


def main() -> int:
    if len(sys.argv) != 2:
        return 1

    api_key = os.getenv("HELP_AI_API_KEY", "").strip()
    if not api_key:
        return 1

    input_path = sys.argv[1]
    try:
        with open(input_path, "r", encoding="utf-8") as handle:
            game_state = handle.read().strip()
    except OSError:
        return 1

    payload = {
        "model": os.getenv("HELP_AI_MODEL", "").strip(),
        "messages": [
            {"role": "system", "content": SYSTEM_PROMPT},
            {
                "role": "user",
                "content": (
                    "当前玩家状态：\n"
                    f"{game_state}\n"
                    "请只回复一条简短建议。"
                ),
            },
        ],
        "stream": False,
    }

    request = urllib.request.Request(
        os.getenv("HELP_AI_API_URL", "").strip(),
        data=json.dumps(payload).encode("utf-8"),
        headers={
            "Content-Type": "application/json",
            "Authorization": f"Bearer {api_key}",
        },
        method="POST",
    )

    try:
        with urllib.request.urlopen(request, timeout=20) as response:
            body = json.loads(response.read().decode("utf-8"))
    except (urllib.error.URLError, urllib.error.HTTPError, TimeoutError, json.JSONDecodeError):
        return 1

    try:
        content = body["choices"][0]["message"]["content"].strip()
    except (KeyError, IndexError, AttributeError, TypeError):
        return 1

    if not content:
        return 1

    print(content)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
