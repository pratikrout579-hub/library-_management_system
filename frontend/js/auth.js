import { api } from './api.js';
import { toast } from './common.js';

const form=document.querySelector('#login-form');
if(form){form.addEventListener('submit',async event=>{event.preventDefault();const button=form.querySelector('button');const error=document.querySelector('#auth-error');error.textContent='';button.disabled=true;button.textContent='Signing in…';try{const values=Object.fromEntries(new FormData(form));const data=await api.login(values.email,values.password);localStorage.setItem('library_token',data.token);localStorage.setItem('library_user',JSON.stringify(data));location.href=data.role==='ADMIN'?'admin/dashboard.html':'student/dashboard.html';}catch(reason){error.textContent=reason.message;toast(reason.message,true);}finally{button.disabled=false;button.textContent='Sign in';}});}
