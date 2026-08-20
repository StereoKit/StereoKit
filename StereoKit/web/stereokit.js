// The StereoKit half of a browser app's startup. A page supplies its canvas
// and, if it wants them, status messages; everything below is StereoKit's own
// contract with the runtime and has no business living in an app's page.
//
// Nothing here is specific to one app, so an app's main.js should only need:
//
//   import { dotnet }  from './_framework/dotnet.js'
//   import { skRun }   from './stereokit.js'
//   await skRun(dotnet, { canvas: document.getElementById('canvas') });

// platform_init can't block waiting for a device, so one is acquired up front
// and parked on the module for it to pick up. A null device is left for
// platform_init to report as a StereoKit failure, rather than trapping in JS.
async function acquireDevice(say) {
	if (!navigator.gpu) {
		say('WebGPU is not available in this browser');
		return null;
	}
	const adapter = await navigator.gpu.requestAdapter();
	if (!adapter) {
		say('navigator.gpu.requestAdapter returned null');
		return null;
	}

	// Firefox leaves most of adapter.info blank, so this can come out empty.
	const info = adapter.info || {};
	const name = [info.vendor, info.architecture, info.device, info.description]
		.filter(s => s).join(' / ');
	say('adapter: ' + (name || 'unnamed'));

	// Take everything the adapter offers, sk_renderer probes for what it got.
	return await adapter.requestDevice({ requiredFeatures: Array.from(adapter.features) });
}

// Media formats the asset system reads through its own task pipeline, which
// streams a missing file from the server on demand. Everything else is read
// synchronously at its call site (shaders, fonts, glTF sidecar files), so it
// has to be in the wasm filesystem before Main runs. Mirrors the formats
// StereoKitC loads through platform_read_file_async, a format added there
// belongs here too or it just preloads.
const skStreamedAsset = /\.(png|jpe?g|hdr|qoi|ktx2|glb|stl|ply)$/i;

// The wasm filesystem starts empty and a browser can't list a directory, so the
// build writes a manifest of everything it published under assetsFolder.
async function loadAssets(FS, assetsFolder, manifestName, say) {
	const manifest = await fetch(assetsFolder + '/' + manifestName);
	if (!manifest.ok) throw new Error('no ' + assetsFolder + '/' + manifestName + ' (' + manifest.status + ')');
	const names   = (await manifest.text()).split('\n').map(n => n.trim()).filter(n => n);
	const preload = names.filter(n => !skStreamedAsset.test(n));

	FS.mkdirTree('/' + assetsFolder);
	await Promise.all(preload.map(async name => {
		const file = await fetch(assetsFolder + '/' + name);
		if (!file.ok) { console.warn('[sk] missing asset ' + name); return; }

		const path = '/' + assetsFolder + '/' + name;
		const dir  = path.substring(0, path.lastIndexOf('/'));
		FS.mkdirTree(dir);
		FS.writeFile(path, new Uint8Array(await file.arrayBuffer()));
	}));
	say('preloaded ' + preload.length + ' of ' + names.length + ' assets, the rest stream on demand');
}

/// Starts a StereoKit app. Returns once Main returns, which on the web is as
/// soon as SK.Run has handed the frame loop to the browser.
///
/// options.canvas       - the canvas to render into, required.
/// options.assetsFolder - defaults to 'Assets', match SKSettings.assetsFolder.
/// options.manifest     - defaults to 'sk_assets.txt', match $(SKWebManifest).
/// options.onStatus     - called with progress messages, optional.
export async function skRun(dotnet, options = {}) {
	const canvas       = options.canvas;
	const assetsFolder = options.assetsFolder || 'Assets';
	const manifestName = options.manifest     || 'sk_assets.txt';
	const say          = (msg) => {
		console.log('[sk] ' + msg);
		if (options.onStatus) options.onStatus(msg);
	};

	// Right-drag is mouse-look, so the context menu has to go.
	canvas.addEventListener('contextmenu', e => e.preventDefault());

	const api = await dotnet
		.withModuleConfig({
			canvas:                     canvas,
			preinitializedWebGPUDevice: await acquireDevice(say),
			print:    (text) => console.log(text),
			printErr: (text) => console.error(text),
		})
		.create();

	await loadAssets(api.Module.FS, assetsFolder, manifestName, say);

	say('starting StereoKit');
	await api.runMain();
	return api;
}
