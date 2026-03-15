#!/usr/bin/env python3

import subprocess
import sys
from pathlib import Path

# Colores para la terminal del Mac
RED = '\033[91m'
YELLOW = '\033[33m'
BOLD = '\033[1m'
RESET = '\033[0m'

# Configuración del estanque
ALTURA_SENSOR = 160  # cm desde el fondo del estanque
CAPACIDAD_LITROS = 5000  # litros cuando está lleno

# Directorio donde está este script
SCRIPT_DIR = Path(__file__).parent

def calcular_nivel(distancia_sensor: float) -> tuple[float, float]:
    """Calcula los litros y porcentaje basándose en la distancia del sensor."""
    altura_agua = ALTURA_SENSOR - distancia_sensor
    if altura_agua < 0:
        altura_agua = 0
    if altura_agua > ALTURA_SENSOR:
        altura_agua = ALTURA_SENSOR
    porcentaje = (altura_agua / ALTURA_SENSOR) * 100
    litros = (altura_agua / ALTURA_SENSOR) * CAPACIDAD_LITROS
    return litros, porcentaje

def main():
    print("🚀 Iniciando monitoreo de la Tinaja...")
    print(f"   Sensor a {ALTURA_SENSOR} cm | Capacidad: {CAPACIDAD_LITROS} L\n")
    
    mqtt_script = SCRIPT_DIR / 'mqtt_read_queue.py'
    cmd = ['python3', str(mqtt_script)]
    
    try:
        proceso = subprocess.Popen(cmd, stdout=subprocess.PIPE, text=True, bufsize=1)
        
        for linea in proceso.stdout:
            linea = linea.strip()
            if not linea:
                continue
                
            # Tu formato: YUS-0.2.8-COSTA,OKO,88.75,2026-03-07...
            partes = linea.split(',')
            
            if len(partes) >= 3 and "OKO" in partes[1]:
                try:
                    # El valor es la distancia desde el sensor hasta el agua (cm)
                    distancia = float(partes[2])
                    litros, porcentaje = calcular_nivel(distancia)
                    
                    # Calcular gotas dinámicas (0-10 basado en porcentaje)
                    num_gotas = round(porcentaje / 10)
                    gotas = " ".join(["💧"] * num_gotas + ["⚪"] * (10 - num_gotas))
                    emoji_nivel = gotas
                    nivel_info = f" → {BOLD}{litros:>5.0f}L ({porcentaje:>3.0f}%) {emoji_nivel}{RESET}"
                    
                    if distancia > 140:
                        print(f"{RED}🚨 PELIGRO:  {linea}{nivel_info}")
                    elif distancia > 80:
                        print(f"{YELLOW}⚠️ ALERTA:   {linea}{nivel_info}")
                    else:
                        print(f"✅ Normal:   {linea}{nivel_info}")
                except ValueError:
                    print(linea)
            else:
                print(linea)
                
    except KeyboardInterrupt:
        print("\n👋 Monitoreo detenido.")
        proceso.terminate()
    except Exception as e:
        print(f"❌ Error: {e}")

if __name__ == '__main__':
    main()
