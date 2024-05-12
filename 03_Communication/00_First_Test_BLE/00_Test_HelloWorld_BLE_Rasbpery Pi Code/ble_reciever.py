from bluepy.btle import UUID, Peripheral

# UUID de la caractéristique que nous voulons lire
SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"

# Adresse MAC du périphérique ESP32 à connecter
DEVICE_MAC = "E8:68:E7:29:4C:8A"  # Remplacez cela par l'adresse MAC de votre ESP32
print("debut")
try:
    # Se connecter au périphérique ESP32
    peripheral = Peripheral(DEVICE_MAC)
    print("connection")
    # Récupérer le service et la caractéristique
    service = peripheral.getServiceByUUID(UUID(SERVICE_UUID))
    characteristic = service.getCharacteristics(UUID(CHARACTERISTIC_UUID))[0]
    print("recuperation du service")
    # Lire la valeur de la caractéristique
    value = characteristic.read()

    # Afficher la valeur lue
    print("Valeur de la caractéristique:", value)

    # Fermer la connexion avec le périphérique
    peripheral.disconnect()
    print("déconection")

except Exception as e:
    print("Erreur:", e)
