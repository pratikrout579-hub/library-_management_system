import { api } from './api.js';

export const escapeHtml = value => String(value ?? '').replace(/[&<>'"]/g, char => ({ '&':'&amp;','<':'&lt;','>':'&gt;',"'":'&#39;','"':'&quot;' }[char]));
export const titleCase = value => String(value || '').replace(/_/g, ' ').toLowerCase().replace(/\b\w/g, letter => letter.toUpperCase());
export const badge = status => `<span class="badge ${String(status).toLowerCase()}">${escapeHtml(status)}</span>`;
export const items = result => result?.items || [];
export const today = () => new Date().toISOString().slice(0, 10);

export function toast(message, error = false) { const area=document.querySelector('.toast-area') || Object.assign(document.body.appendChild(document.createElement('div')),{className:'toast-area'}); const note=Object.assign(document.createElement('div'),{className:`toast${error?' error':''}`,textContent:message});area.append(note);setTimeout(()=>note.remove(),4200); }
export function setLoading(target, message='Loading…') { target.innerHTML=`<div class="loading">${message}</div>`; }
export function empty(message='Nothing to show yet.') { return `<div class="empty"><span class="symbol">⌁</span>${escapeHtml(message)}</div>`; }
export function showModal(title, content, onSubmit, submitText='Save') {
  const backdrop=document.createElement('div');backdrop.className='modal-backdrop';backdrop.innerHTML=`<section class="modal" role="dialog" aria-modal="true"><div class="modal-head"><h3>${escapeHtml(title)}</h3><button class="close" aria-label="Close">×</button></div><form>${content}<div class="form-actions"><button type="button" class="button secondary cancel">Cancel</button><button class="button submit" type="submit">${escapeHtml(submitText)}</button></div></form></section>`;document.body.append(backdrop);
  const close=()=>backdrop.remove(); backdrop.querySelectorAll('.close,.cancel').forEach(button=>button.onclick=close);backdrop.onclick=event=>{if(event.target===backdrop)close();};backdrop.querySelector('form').onsubmit=async event=>{event.preventDefault();const submit=backdrop.querySelector('.submit');submit.disabled=true;submit.textContent='Saving…';try{await onSubmit(Object.fromEntries(new FormData(event.currentTarget)));close();}catch(error){toast(error.message,true);submit.disabled=false;submit.textContent=submitText;}};
}
export function confirm(title, text, action) { showModal(title, `<p class="confirm-copy">${escapeHtml(text)}</p>`, async()=>{await action();}, 'Confirm'); }
export function formField(name,label,value='',type='text',full=false,required=true) { return `<label class="${full?'full':''}">${label}<input class="field" type="${type}" name="${name}" value="${escapeHtml(value)}" ${required?'required':''}></label>`; }
export function selectField(name,label,options,value='',full=false) { return `<label class="${full?'full':''}">${label}<select class="field" name="${name}">${options.map(option=>`<option value="${escapeHtml(option)}" ${String(option)===String(value)?'selected':''}>${escapeHtml(titleCase(option))}</option>`).join('')}</select></label>`; }
const nav = {
  admin:[['dashboard','Dashboard','◫'],['books','Books','▤'],['students','Students','♙'],['requests','Book Requests','◌'],['issue','Issue Book','↗'],['return','Return Book','↙'],['transactions','Transactions','↹'],['reports','Reports','◈'],['profile','Profile','◉']],
  student:[['dashboard','Dashboard','◫'],['books','Browse Books','▤'],['requests','My Requests','◌'],['my-books','My Books','↗'],['history','History','↹'],['profile','Profile','◉']]
};
export async function mountPortal(role) {
  const user=await api.me().catch(()=>null);if(!user || user.role !== role.toUpperCase()) { localStorage.clear();location.href=role==='admin'?'../login.html':'../login.html';return null; }
  const page=document.body.dataset.page;document.body.innerHTML=`<div class="app-shell"><aside class="sidebar"><a class="brand" href="dashboard.html"><span class="brand-mark">L</span><span>Smart Library<small>${titleCase(role)} portal</small></span></a><nav><div class="nav-label">Workspace</div>${nav[role].map(([id,label,icon])=>`<a class="nav-link ${page===id?'active':''}" href="${id}.html"><b>${icon}</b><span>${label}</span></a>`).join('')}<a class="nav-link logout" href="#logout" id="logout"><b>↪</b><span>Logout</span></a></nav></aside><main class="main"><header class="topbar"><div><h1 id="top-title">Smart Library</h1><p>College library operations and discovery</p></div><div class="user-chip"><span>${escapeHtml(user.name)}</span><div class="avatar">${escapeHtml(user.name[0])}</div></div></header><div class="content" id="content"></div></main></div>`;
  document.querySelector('#logout').onclick=async event=>{event.preventDefault();try{await api.logout();}finally{localStorage.clear();location.href='../login.html';}};return user;
}
export function heading(title, subtitle='', action='') { document.querySelector('#top-title').textContent=title; return `<div class="page-heading"><div><h2>${escapeHtml(title)}</h2><p>${escapeHtml(subtitle)}</p></div>${action}</div>`; }
export function stat(label,value,icon='◫') { return `<article class="card stat"><span class="stat-label">${escapeHtml(label)}</span><div class="stat-value">${escapeHtml(value)}</div><span class="stat-icon">${icon}</span></article>`; }
export function table(headers, rows, emptyText) { return `<div class="card table-wrap">${rows.length?`<table class="data-table"><thead><tr>${headers.map(header=>`<th>${escapeHtml(header)}</th>`).join('')}</tr></thead><tbody>${rows.join('')}</tbody></table>`:empty(emptyText)}</div>`; }
