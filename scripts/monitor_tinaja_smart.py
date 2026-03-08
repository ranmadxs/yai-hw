#!/usr/bin/env python3
import subprocess
import sys
import json
import urllib.request
from pathlib import Path

# Colores para la terminal
RED = '\033[91m'
YELLOW = '\033[33m'
CYAN = '\033[96m'
BOLD = '\033[1m'
RESET = '\033[0m'

SCRIPT_DIR = Path(__file__).parent
OLLAMA_URL = "http://localhost:11434/v1/chat/completions"
MODEL = "llama3.1:8b"

def ask_ollama(prompt: str) -> str:
    """Consulta a Ollama y devuelve la respuesta."""
    try:
        data = json.dumps({
            "model": MODEL,
            "messages": [{"role": "user", "content": prompt}]
        }).encode('utf-8')
        
        req = urllib.request.Request(
            OLLAMA_URL,
            data=data,
            headers={"Content-Type": "application/json"}
        )
        
        with urllib.request.urlopen(req, timeout=30) as resp:
            result = json.loads(resp.read().decode('utf-8'))
            return result['choices'][0]['message']['content']
    except Exception as e:
        return f"Error consultando Ollama: {e}"

def main():
    print(f"{BOLD}🚀 Monitor Tinaja SMART (con IA){RESET}")
    print(f"   Modelo: {MODEL}\n")
    
    mqtt_script = SCRIPT_DIR / 'mqtt_read_queue.py'
    cmd = ['python3', str(mqtt_script)]
    
    try:
        proceso = subprocess.Popen(cmd, stdout=subprocess.PIPE, text=True, bufsize=1)
        
        for linea in proceso.stdout:
            linea = linea.strip()
            if not linea:
                continue
            
            partes = linea.split(',')
            
            if len(partes) >= 3 and "OKO" in partes[1]:
                try:
                    valor = float(partes[2])
                    timestamp = partes[3] if len(partes) > 3 else ""
                    
                    if valor > 160:
                        print(f"{RED}🚨 PELIGRO: {linea}{RESET}")
                        print(f"{CYAN}🤖 SGPT ALERT:{RESET}")
                        respuesta = ask_ollama(
                            f"ALERTA CRÍTICA: El sensor de la tinaja marca {valor}. "
                            f"El umbral de peligro es 160. Dame una recomendación urgente en 1-2 líneas."
                        )
                        print(f"   {respuesta}\n")
                        
                    elif valor > 80:
                        print(f"{YELLOW}⚠️ ALERTA: {linea}{RESET}")
                        print(f"{CYAN}🤖 SGPT ALERT:{RESET}")
                        respuesta = ask_ollama(
                            f"ADVERTENCIA: El sensor de la tinaja marca {valor}. "
                            f"El umbral de alerta es 80. Dame un consejo breve en 1 línea."
                        )
                        print(f"   {respuesta}\n")
                        
                    else:
                        print(f"✅ Normal: {linea}")
                        
                except ValueError:
                    print(linea)
            else:
                print(linea)
                
    except KeyboardInterrupt:
        print(f"\n{BOLD}👋 Monitor detenido.{RESET}")
        proceso.terminate()
    except Exception as e:
        print(f"❌ Error: {e}")

if __name__ == '__main__':
    main()
