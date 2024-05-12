import tkinter as tk
from tkinter import ttk, messagebox
import bluetooth
import socket
import threading

class BluetoothApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Communication Bluetooth")

        self.container = tk.Frame(self)
        self.container.pack(side="top", fill="both", expand=True)

        self.pages = {}
        for PageClass in (DiscoverPage, CommunicationPage):
            page_name = PageClass.__name__
            page = PageClass(self.container, self)
            self.pages[page_name] = page
            page.grid(row=0, column=0, sticky="nsew")

        self.show_page("DiscoverPage")

    def show_page(self, page_name):
        page = self.pages[page_name]
        page.tkraise()


class DiscoverPage(tk.Frame):
    def __init__(self, parent, controller):
        super().__init__(parent)
        self.controller = controller

        label_discover = tk.Label(self, text="Liste des appareils Bluetooth à proximité:")
        label_discover.grid(row=0, column=0, columnspan=2)

        self.devices_tree = ttk.Treeview(self, columns=("Nom", "Adresse MAC"), show="headings")
        self.devices_tree.heading("Nom", text="Nom")
        self.devices_tree.heading("Adresse MAC", text="Adresse MAC")
        self.devices_tree.grid(row=1, column=0, columnspan=2)
        self.devices_tree.bind("<Double-1>", self.on_device_select)  # Bind de l'événement Double-Click

        self.discover_button = tk.Button(self, text="Découvrir les périphériques", command=self.discover_devices)
        self.discover_button.grid(row=2, column=0, columnspan=2)

    def discover_devices(self):
        self.discover_button.config(state="disabled")
        threading.Thread(target=self._discover_devices).start()

    def _discover_devices(self):
        nearby_devices = bluetooth.discover_devices(duration=8, lookup_names=True, device_id=-1)
        self.discover_button.config(state="normal")

        if not nearby_devices:
            messagebox.showinfo("Aucun périphérique", "Aucun périphérique Bluetooth trouvé.")
            return

        for device in nearby_devices:
            self.devices_tree.insert("", "end", values=device)

        self.controller.pages["CommunicationPage"].update_devices(nearby_devices)

    def on_device_select(self, event):
        item = self.devices_tree.selection()[0]
        selected_device = self.devices_tree.item(item)['values']
        self.controller.pages["CommunicationPage"].selected_device = selected_device
        self.controller.show_page("CommunicationPage")


class CommunicationPage(tk.Frame):
    def __init__(self, parent, controller):
        super().__init__(parent)
        self.controller = controller

        self.selected_device = None

        message_label = tk.Label(self, text="Message à envoyer:")
        message_label.grid(row=0, column=0)

        self.message_entry = tk.Entry(self)
        self.message_entry.grid(row=0, column=1)

        self.send_button = tk.Button(self, text="Envoyer", command=self.communicate_with_device)
        self.send_button.grid(row=0, column=2)

        self.conversation_text = tk.Text(self, height=10, width=50)
        self.conversation_text.grid(row=1, columnspan=3)

    def update_devices(self, devices):
        self.devices = devices

    def communicate_with_device(self):
        if not self.selected_device:
            messagebox.showerror("Erreur", "Aucun périphérique sélectionné.")
            return

        try:
            sock = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
            sock.connect((self.selected_device[0], 1))
            message = self.message_entry.get()
            if message.lower() == "exit":
                return
            sock.send(message.encode())
            received_data = sock.recv(1024)
            self.conversation_text.insert(tk.END, f"Envoyé: {message}\nRéponse: {received_data.decode()}\n\n")
        except Exception as e:
            messagebox.showerror("Erreur", f"Erreur lors de la communication avec le périphérique : {e}")
        finally:
            sock.close()


if __name__ == "__main__":
    app = BluetoothApp()
    app.mainloop()
