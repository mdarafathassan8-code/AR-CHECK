const { app, BrowserWindow, WebContentsView, ipcMain, session, dialog } = require('electron');
const path = require('path');

let win;
const tabs = new Map();
let activeId = null;
let nextId = 1;
const HOME = 'file://' + path.join(__dirname, 'renderer', 'home.html');

function makeTab(url = HOME) {
  const id = nextId++;
  const view = new WebContentsView({
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      sandbox: true
    }
  });
  tabs.set(id, { id, view, url });
  view.webContents.setWindowOpenHandler(({ url }) => {
    makeTab(url);
    return { action: 'deny' };
  });
  view.webContents.on('did-navigate', () => sendState(id));
  view.webContents.on('did-navigate-in-page', () => sendState(id));
  view.webContents.on('page-title-updated', () => sendState(id));
  view.webContents.on('did-finish-load', () => sendState(id));
  view.webContents.loadURL(url);
  return id;
}

function sendState(id) {
  const t = tabs.get(id);
  if (!t || !win || win.isDestroyed()) return;
  win.webContents.send('tab-state', {
    id,
    title: t.view.webContents.getTitle() || 'New Tab',
    url: t.view.webContents.getURL(),
    canGoBack: t.view.webContents.canGoBack(),
    canGoForward: t.view.webContents.canGoForward(),
    active: id === activeId
  });
}

function showTab(id) {
  const t = tabs.get(id);
  if (!t) return;
  if (activeId && tabs.has(activeId)) win.contentView.removeChildView(tabs.get(activeId).view);
  activeId = id;
  win.contentView.addChildView(t.view);
  layout();
  for (const key of tabs.keys()) sendState(key);
}

function closeTab(id) {
  const t = tabs.get(id);
  if (!t) return;
  const wasActive = activeId === id;
  if (wasActive) win.contentView.removeChildView(t.view);
  t.view.webContents.close();
  tabs.delete(id);
  if (!tabs.size) {
    const newId = makeTab();
    showTab(newId);
  } else if (wasActive) {
    showTab([...tabs.keys()][Math.max(0, [...tabs.keys()].indexOf(id) - 1)] || [...tabs.keys()][0]);
  }
  win.webContents.send('tabs-reset');
  for (const key of tabs.keys()) sendState(key);
}

function layout() {
  if (!win || !activeId || !tabs.has(activeId)) return;
  const bounds = win.getContentBounds();
  tabs.get(activeId).view.setBounds({ x: 0, y: 112, width: bounds.width, height: Math.max(0, bounds.height - 112) });
}

function createWindow() {
  win = new BrowserWindow({ width: 1400, height: 900, minWidth: 900, minHeight: 600, title: 'Nova Browser', webPreferences: { preload: path.join(__dirname, 'preload.js'), contextIsolation: true, sandbox: true } });
  win.loadFile(path.join(__dirname, 'renderer', 'chrome.html'));
  win.on('resize', layout);
  win.on('closed', () => { for (const t of tabs.values()) t.view.webContents.close(); });
  win.webContents.once('did-finish-load', () => { const id = makeTab(); showTab(id); });
}

app.whenReady().then(createWindow);
app.on('window-all-closed', () => { if (process.platform !== 'darwin') app.quit(); });
app.on('activate', () => { if (!BrowserWindow.getAllWindows().length) createWindow(); });

ipcMain.handle('new-tab', (_, url) => { const id = makeTab(url || HOME); showTab(id); return id; });
ipcMain.on('switch-tab', (_, id) => showTab(Number(id)));
ipcMain.on('close-tab', (_, id) => closeTab(Number(id)));
ipcMain.on('navigate', (_, value) => {
  const t = tabs.get(activeId); if (!t) return;
  let target = String(value || '').trim();
  if (!target) return;
  if (!/^https?:\/\//i.test(target)) {
    if (/^[\w.-]+\.[a-z]{2,}(\/.*)?$/i.test(target)) target = 'https://' + target;
    else target = 'https://www.google.com/search?q=' + encodeURIComponent(target);
  }
  t.view.webContents.loadURL(target);
});
ipcMain.on('back', () => { const t=tabs.get(activeId); if(t?.view.webContents.canGoBack()) t.view.webContents.goBack(); });
ipcMain.on('forward', () => { const t=tabs.get(activeId); if(t?.view.webContents.canGoForward()) t.view.webContents.goForward(); });
ipcMain.on('reload', () => tabs.get(activeId)?.view.webContents.reload());
ipcMain.handle('get-tabs', () => [...tabs.values()].map(t => ({ id:t.id, title:t.view.webContents.getTitle() || 'New Tab', url:t.view.webContents.getURL(), active:t.id===activeId })));
ipcMain.handle('install-extension', async () => {
  const result = await dialog.showOpenDialog(win, { properties:['openDirectory'], title:'Select unpacked Chrome extension folder' });
  if (result.canceled || !result.filePaths[0]) return { ok:false };
  try {
    const ext = await session.defaultSession.loadExtension(result.filePaths[0], { allowFileAccess: true });
    return { ok:true, name:ext.name, id:ext.id };
  } catch (e) { return { ok:false, error:e.message }; }
});
ipcMain.handle('active-state', () => { const t=tabs.get(activeId); return t ? {id:activeId,url:t.view.webContents.getURL(),title:t.view.webContents.getTitle()} : null; });
