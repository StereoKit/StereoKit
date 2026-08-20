import { dotnet } from './_framework/dotnet.js'
import { skRun }  from './stereokit.js'

const canvas  = document.getElementById('canvas');
const overlay = document.getElementById('sk-overlay');
const status  = document.getElementById('sk-status');
const hint    = document.getElementById('sk-hint');

// The hint has done its job once you've actually done the thing
const dropHint = () => {
	hint.classList.add('hidden');
	removeEventListener('keydown',   dropHint);
	removeEventListener('mousedown', dropHint);
};
addEventListener('keydown',   dropHint);
addEventListener('mousedown', dropHint);
setTimeout(dropHint, 12000);

const fail = (html) => {
	overlay.classList.remove('hidden');
	overlay.classList.add('error');
	status.innerHTML = html;
};

// The wasm side reports this too, but a plain message beats a spinner that
// never stops.
if (!navigator.gpu) {
	fail('This page needs <a href="https://caniuse.com/webgpu">WebGPU</a>, which this browser does not have enabled.');
} else {
	try {
		await skRun(dotnet, { canvas, onStatus: msg => status.textContent = msg });
		overlay.classList.add('hidden');
		// Keyboard input needs the canvas focused, and the browser won't do it
		canvas.focus();
	} catch (e) {
		fail('Failed to start: ' + e);
		throw e;
	}
}
