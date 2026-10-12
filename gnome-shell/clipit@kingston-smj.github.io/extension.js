import Clutter from 'gi://Clutter';
import Gio from 'gi://Gio';
import GLib from 'gi://GLib';
import Meta from 'gi://Meta';
import St from 'gi://St';

import {Extension} from 'resource:///org/gnome/shell/extensions/extension.js';

const APP_SERVICE = 'org.clipit.ClipIt';
const APP_PATH = '/org/clipit/Bridge';
const APP_INTERFACE = 'org.clipit.Bridge';
const SHELL_PATH = '/org/clipit/Shell';
const MAX_TEXT_BYTES = 64 * 1024;
const MAX_PNG_BYTES = 4 * 1024 * 1024;

const INTERFACE_XML = `<node>
  <interface name="org.clipit.Shell">
    <method name="Paste"/>
  </interface>
</node>`;

export default class ClipItBridge extends Extension {
    enable() {
        this._clipboard = St.Clipboard.get_default();
        this._selection = global.display.get_selection();
        this._selectionId = this._selection.connect('owner-changed',
            (_selection, type) => {
                if (type === Meta.SelectionType.SELECTION_CLIPBOARD)
                    this._capture();
            });
        this._exported = Gio.DBusExportedObject.wrapJSObject(INTERFACE_XML, this);
        this._exported.export(Gio.DBus.session, SHELL_PATH);
        this._keyboard = null;
        this._pasteTimer = 0;
        this._enabled = true;
        this._send('BridgeReady', null);
    }

    disable() {
        this._send('BridgeStopping', null);
        this._enabled = false;
        if (this._pasteTimer) {
            GLib.Source.remove(this._pasteTimer);
            this._pasteTimer = 0;
        }
        if (this._selectionId) {
            this._selection.disconnect(this._selectionId);
            this._selectionId = 0;
        }
        this._exported?.unexport();
        this._exported = null;
        this._keyboard = null;
        this._clipboard = null;
        this._selection = null;
    }

    _capture() {
        const types = this._clipboard.get_mimetypes(St.ClipboardType.CLIPBOARD) ?? [];
        if (types.includes('image/png')) {
            this._clipboard.get_content(St.ClipboardType.CLIPBOARD, 'image/png',
                (_clipboard, bytes) => {
                    if (!this._enabled || !bytes)
                        return;
                    const data = bytes.get_data();
                    if (data.length > 0 && data.length <= MAX_PNG_BYTES)
                        this._send('CapturePng', new GLib.Variant('(ay)', [data]));
                });
            return;
        }

        if (types.some(type => type === 'text/plain' ||
                type.startsWith('text/plain;'))) {
            this._clipboard.get_text(St.ClipboardType.CLIPBOARD,
                (_clipboard, text) => {
                    if (!this._enabled || !text)
                        return;
                    if (new TextEncoder().encode(text).length <= MAX_TEXT_BYTES)
                        this._send('CaptureText', new GLib.Variant('(s)', [text]));
                });
        }
    }

    _send(method, parameters) {
        Gio.DBus.session.call(APP_SERVICE, APP_PATH, APP_INTERFACE,
            method, parameters, null, Gio.DBusCallFlags.NO_AUTO_START,
            1000, null, (_connection, result) => {
                try {
                    Gio.DBus.session.call_finish(result);
                } catch (e) {
                    // ClipIt may not be running yet. The next copy is retried.
                }
            });
    }

    Paste() {
        if (this._pasteTimer)
            GLib.Source.remove(this._pasteTimer);
        // The Qt popup closes immediately after selection. A one-shot delay
        // lets the previously focused application receive keyboard focus.
        this._pasteTimer = GLib.timeout_add(GLib.PRIORITY_DEFAULT, 150, () => {
            this._pasteTimer = 0;
            if (this._enabled)
                this._sendPasteKeys();
            return GLib.SOURCE_REMOVE;
        });
    }

    _sendPasteKeys() {
        if (!this._keyboard) {
            const seat = Clutter.get_default_backend().get_default_seat();
            this._keyboard = seat.create_virtual_device(
                Clutter.InputDeviceType.KEYBOARD_DEVICE);
        }
        const pressed = Clutter.KeyState.PRESSED;
        const released = Clutter.KeyState.RELEASED;
        const ctrl = 29; // Linux KEY_LEFTCTRL
        const v = 47;    // Linux KEY_V
        this._keyboard.notify_key(GLib.get_monotonic_time(), ctrl, pressed);
        this._keyboard.notify_key(GLib.get_monotonic_time(), v, pressed);
        this._keyboard.notify_key(GLib.get_monotonic_time(), v, released);
        this._keyboard.notify_key(GLib.get_monotonic_time(), ctrl, released);
    }
}
