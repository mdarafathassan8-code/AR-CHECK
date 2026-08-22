const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('nova', {
  newTab: (url) => ipcRenderer.invoke('new-tab', url),
  switchTab: (id) => ipcRenderer.send('switch-tab', id),
  closeTab: (id) => ipcRenderer.send('close-tab', id),
  navigate: (value) => ipcRenderer.send('navigate', value),
  back: () => ipcRenderer.send('back'),
  forward: () => ipcRenderer.send('forward'),
  reload: () => ipcRenderer.send('reload'),
  getTabs: () => ipcRenderer.invoke('get-tabs'),
  installExtension: () => ipcRenderer.invoke('install-extension'),
  activeState: () => ipcRenderer.invoke('active-state'),
  onTabState: (fn) => ipcRenderer.on('tab-state', (_, data) => fn(data)),
  onTabsReset: (fn) => ipcRenderer.on('tabs-reset', () => fn())
});
