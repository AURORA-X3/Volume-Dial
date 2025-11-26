import time
import serial
from pycaw.pycaw import AudioUtilities
import serial.tools.list_ports


SIGNATURE = "ESP-SERVER"

def find_esp_server():
    print("Scan des ports série...")

    while True:
        ports = serial.tools.list_ports.comports()

        for p in ports:
            try:
                ser = serial.Serial(p.device, 115200, timeout=0.5)
                time.sleep(1)

                for _ in range(5):
                    if ser.in_waiting:
                        resp = ser.readline().decode(errors="ignore").strip()

                        print(resp)

                        # Si le port répond EXACTEMENT "ESP-SERVER"
                        if resp == SIGNATURE:
                            print("ESP serveur détecté sur", p.device)
                            return ser

                    time.sleep(0.1)

                ser.close()

            except Exception:
                pass

        # si aucun port ne correspond, on recommence le scan
        print("Aucun port valide trouvé, nouvelle tentative...")
        time.sleep(1)

# Setup volume control
devices = AudioUtilities.GetSpeakers()
volume = devices.EndpointVolume

def get_pc_volume():
    return round(volume.GetMasterVolumeLevelScalar()*100)

def set_pc_volume(v):
    v = max(0, min(1, v))
    volume.SetMasterVolumeLevelScalar(v, None)

ser = find_esp_server()

pc_vol = get_pc_volume()
ser.write(f"{pc_vol}\n".encode())

time.sleep(2)

last_volume = pc_vol

print("Python <-> ESP32 Serveur bridge prêt")




while True:
    # PC → ESP (envoie si volume change)
    pc_vol = get_pc_volume()

    if abs(pc_vol - last_volume) > 1:
        print(pc_vol)
        ser.write(f"{pc_vol}\n".encode())
        last_volume = pc_vol

    # ESP → Python (commande SET:xxx venant de l’ESP client)
    if ser.in_waiting:
        msg = ser.readline().decode().strip()

        if msg.startswith("SET:"):
            try:
                v = float(msg.split(":")[1])/100
                set_pc_volume(v)
                last_volume = v
            except:
                pass

    time.sleep(0.05)
