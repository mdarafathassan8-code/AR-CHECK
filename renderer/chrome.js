const tabsEl=document.getElementById('tabs'), address=document.getElementById('address');
let tabState=new Map();
function renderTabs(){tabsEl.innerHTML='';for(const t of tabState.values()){const el=document.createElement('div');el.className='tab '+(t.active?'active':'');el.onclick=()=>nova.switchTab(t.id);const title=document.createElement('span');title.textContent=t.title||'New Tab';const close=document.createElement('button');close.textContent='×';close.onclick=e=>{e.stopPropagation();nova.closeTab(t.id)};el.append(title,close);tabsEl.appendChild(el)}}
async function refresh(){const list=await nova.getTabs();tabState=new Map(list.map(t=>[t.id,t]));renderTabs();const active=list.find(t=>t.active);if(active)address.value=active.url&&active.url!=='file:///home' ? active.url : ''}
document.getElementById('form').onsubmit=e=>{e.preventDefault();nova.navigate(address.value)};
document.getElementById('back').onclick=()=>nova.back();document.getElementById('forward').onclick=()=>nova.forward();document.getElementById('reload').onclick=()=>nova.reload();document.getElementById('home').onclick=()=>nova.newTab();document.getElementById('plus').onclick=()=>nova.newTab();
document.getElementById('extension').onclick=async()=>{const r=await nova.installExtension();alert(r.ok?`Extension loaded: ${r.name}`:`Could not load extension${r.error?': '+r.error:''}`)};
nova.onTabState(s=>{tabState.set(s.id,{...(tabState.get(s.id)||{}),...s});renderTabs();if(s.active&&s.url)address.value=s.url});nova.onTabsReset(refresh);refresh();
