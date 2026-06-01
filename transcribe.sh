#!/bin/bash

# ============================================================
# Транскрибация видео + скриншоты
# Использование: ./transcribe.sh video.mp4 [интервал_сек]
# Пример:        ./transcribe.sh video.mp4 60
# ============================================================

VIDEO="$1"
INTERVAL="${2:-30}"

# Проверка аргумента
if [ -z "$VIDEO" ]; then
    echo "❌ Укажите файл: ./transcribe.sh video.mp4 [интервал_сек]"
    exit 1
fi

if [ ! -f "$VIDEO" ]; then
    echo "❌ Файл не найден: $VIDEO"
    exit 1
fi

# Папка для результатов
NAME=$(basename "$VIDEO" | sed 's/\.[^.]*$//')
OUTDIR="$PWD/$NAME"
mkdir -p "$OUTDIR/screenshots"

# Длительность видео (секунды) для прогресс-баров
DURATION=$(ffprobe -v error -show_entries format=duration \
    -of default=noprint_wrappers=1:nokey=1 "$VIDEO" 2>/dev/null | awk -F. '{print $1}')

echo "======================================"
echo "📁 Файл:       $VIDEO"
echo "📂 Результаты: $OUTDIR"
printf "⏱ Длительность: %02d:%02d:%02d\n" $((DURATION/3600)) $((DURATION%3600/60)) $((DURATION%60))
echo "======================================"

# ── Скриншоты ──────────────────────────────────────────────
echo ""
echo "📸 Снимаю скриншоты каждые ${INTERVAL} сек..."

ffmpeg -i "$VIDEO" \
    -vf fps=1/$INTERVAL \
    "$OUTDIR/screenshots/frame_%04d.jpg" \
    -loglevel error \
    -progress pipe:1 2>/dev/null | \
while IFS='=' read -r key val; do
    if [ "$key" = "out_time_us" ] && [ -n "$val" ] && [ "$DURATION" -gt 0 ] 2>/dev/null; then
        cur=$((val / 1000000))
        pct=$((cur * 100 / DURATION))
        [ $pct -gt 100 ] && pct=100
        filled=$((pct / 5))
        bar=$(printf '█%.0s' $(seq 1 $filled 2>/dev/null))
        empty=$(printf '░%.0s' $(seq 1 $((20 - filled)) 2>/dev/null))
        printf "\r  [%s%s] %3d%%  %02d:%02d / %02d:%02d" \
            "$bar" "$empty" "$pct" \
            $((cur/60)) $((cur%60)) \
            $((DURATION/60)) $((DURATION%60))
    fi
done
echo ""

echo "✅ Скриншоты готовы: $(ls "$OUTDIR/screenshots" | wc -l) шт."

# ── Транскрибация ───────────────────────────────────────────
echo ""
echo "🎙️  Транскрибирую (модель: medium)..."
echo "    (первый запуск скачает модель ~1.5 GB)"
echo ""

VIDEO_PATH="$VIDEO" OUTDIR_PATH="$OUTDIR" DURATION="$DURATION" python3 - <<'EOF'
import os, sys

video    = os.environ["VIDEO_PATH"]
out_dir  = os.environ["OUTDIR_PATH"]
duration = float(os.environ.get("DURATION", "0"))

from faster_whisper import WhisperModel

model = WhisperModel("medium", device="cpu", compute_type="int8")
segments, info = model.transcribe(video, language="ru", beam_size=5)

print(f"Язык: {info.language} (уверенность: {info.language_probability:.0%})")
print("")

txt_path = os.path.join(out_dir, "transcript.txt")
srt_path = os.path.join(out_dir, "transcript.srt")

def fmt(t):
    h = int(t // 3600); m = int((t % 3600) // 60); s = int(t % 60); ms = int((t % 1) * 1000)
    return f"{h:02}:{m:02}:{s:02},{ms:03}"

def progress_bar(current, total, width=20):
    if total <= 0:
        return ""
    pct = min(100, int(current / total * 100))
    filled = pct // (100 // width)
    bar = "█" * filled + "░" * (width - filled)
    cur_str = f"{int(current//60):02d}:{int(current%60):02d}"
    tot_str = f"{int(total//60):02d}:{int(total%60):02d}"
    return f"\r  [{bar}] {pct:3d}%  {cur_str} / {tot_str}"

with open(txt_path, "w") as txt, open(srt_path, "w") as srt:
    for i, seg in enumerate(segments, 1):
        # Прогресс
        pb = progress_bar(seg.end, duration)
        if pb:
            print(pb, end="", flush=True)

        line = f"[{seg.start:6.1f}s - {seg.end:6.1f}s]  {seg.text.strip()}"
        txt.write(line + "\n")
        srt.write(f"{i}\n{fmt(seg.start)} --> {fmt(seg.end)}\n{seg.text.strip()}\n\n")

if duration > 0:
    print(f"\r  [{'█'*20}] 100%  {int(duration//60):02d}:{int(duration%60):02d} / {int(duration//60):02d}:{int(duration%60):02d}")
print("")
print(f"💾 Текст сохранён:  {txt_path}")
print(f"💾 Субтитры (.srt): {srt_path}")
EOF

# ── Итог ────────────────────────────────────────────────────
echo ""
echo "======================================"
echo "✅ Всё готово!"
echo "   📂 $OUTDIR"
ls -lh "$OUTDIR/"
echo "   📂 $OUTDIR/screenshots/"
ls "$OUTDIR/screenshots/" | tail -3
echo "======================================"
