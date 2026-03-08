#!/bin/bash
# Monitor Tinaja con SGPT
# Usa el skill tinaja-monitor-skill.md para que sgpt analice los datos

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SKILL_FILE="$SCRIPT_DIR/../skills/tinaja-monitor-skill.md"

echo "🚀 Monitor Tinaja con SGPT"
echo "   Skill: $SKILL_FILE"
echo ""

# Lee el skill una vez
SKILL=$(cat "$SKILL_FILE")

# Ejecuta mqtt_read_queue.py y procesa cada línea
python3 "$SCRIPT_DIR/mqtt_read_queue.py" | while read -r linea; do
    if [ -n "$linea" ]; then
        echo "📊 Dato: $linea"
        
        # Pasa la línea a sgpt con el skill como contexto
        sgpt --no-cache "$SKILL

Analiza esta lectura: $linea"
        
        echo ""
    fi
done
