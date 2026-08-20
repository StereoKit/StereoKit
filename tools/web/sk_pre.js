// StereoKit WebGPU pre-init. main() can't block for a device, so this runs
// first and holds the runtime back until one resolves, parked on
// Module.preinitializedWebGPUDevice for platform_init to pick up.
//
// Two traps: Module property names are quoted so closure can't rename across
// the boundary, and logging uses console rather than Module.print/printErr,
// which are unexported and abort the runtime on read.

// Without these, an uncaught exception kills the frame loop silently.
window.addEventListener('error', function (e) {
	console.error('[sk] uncaught: ' + e.message + ' @ ' + e.filename + ':' + e.lineno);
});
window.addEventListener('unhandledrejection', function (e) {
	console.error('[sk] unhandled rejection: ' + e.reason);
});

Module['preRun'] = Module['preRun'] || [];
Module['preRun'].push(function () {
	var log = function (msg) { console.log('[sk] ' + msg); };
	if (!navigator.gpu) {
		// Leave the device unset. platform_init reports it as a StereoKit
		// failure reason rather than trapping somewhere in JS.
		log('WebGPU is not available in this browser (navigator.gpu missing)');
		return;
	}
	addRunDependency('sk-webgpu-device');
	navigator.gpu.requestAdapter().then(function (adapter) {
		if (!adapter) throw new Error('navigator.gpu.requestAdapter returned null');
		var info = adapter.info || {};
		var name = [info.vendor, info.architecture, info.device, info.description]
			.filter(function (s) { return s; }).join(' / ');
		log('WebGPU adapter: ' + name);
		// emdawnwebgpu can import a device but not an adapter, so this is the
		// only route the C side has to an adapter name.
		Module['skWebGPUAdapterName'] = name || 'WebGPU';
		// Take everything the adapter offers, sk_renderer probes for what it
		// actually got at runtime
		return adapter.requestDevice({ requiredFeatures: Array.from(adapter.features) });
	}).then(function (device) {
		log('WebGPU device ready');
		Module['preinitializedWebGPUDevice'] = device;
		removeRunDependency('sk-webgpu-device');
	}).catch(function (e) {
		log('WebGPU device request failed: ' + e);
		removeRunDependency('sk-webgpu-device');
	});
});
