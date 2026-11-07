import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { pathToFileURL } from 'node:url';
import { URI as VscodeURI } from 'vscode-uri';
import { createTypeScriptLoader, src } from './typescript-loader.mjs';
const URI = { file(file) { return { scheme: 'file', fsPath: path.resolve(file), path: path.resolve(file), toString() { return pathToFileURL(this.fsPath).toString(); } }; } };
const plain = value => JSON.parse(JSON.stringify(value));
const load = createTypeScriptLoader();
const templates = load(src('wizard/file-wizard-templates.ts'));
const discovery = load(src('contributions/discovery.ts'));
const golden = JSON.parse(fs.readFileSync(new URL('./fixtures/starter-files.json',import.meta.url),'utf8'));
for (const [key,expected] of Object.entries(golden)) test('starter file: '+key,()=> {
 const [fileStem,kind]=key.split('/');
 assert.deepEqual(plain(templates.createXsmpStarterFileTemplate(kind,{fileStem,author:'alice',date:'2026-03-27T08:00:00Z'})),expected);
});
test('contribution discovery resolves manifests, aliases, builtin paths and runtime handlers',()=> {
 const manifest={contributes:{xsmp:[{descriptor:'profiles/demo.xsmpprofile',handler:'handler.js',nativeHandler:{[`${process.platform}-${process.arch}`]:'native/worker'},wasiHandler:'portable/worker.wasm',apiVersion:'^2.0.0',aliases:['Demo'],deprecatedAliases:['old-demo'],builtins:['models'],wizard:{label:'Demo Runtime'}}]}};
 const native=discovery.resolveContributionEntries('vendor.demo','/extension',manifest,'native',assert.fail);
 const wasi=discovery.resolveContributionEntries('vendor.demo','/extension',manifest,'wasi',assert.fail);
 assert.equal(native[0].handlerPath,'/extension/native/worker');assert.equal(wasi[0].handlerPath,'/extension/portable/worker.wasm');
 assert.deepEqual(plain(native[0].aliases),['Demo']);assert.deepEqual(plain(native[0].deprecatedAliases),['old-demo']);assert.deepEqual(plain(native[0].builtins),['/extension/models']);assert.equal(native[0].wizard.label,'Demo Runtime');
});
test('contribution discovery falls back on the handler',()=> {
 const entries=discovery.resolveContributionEntries('vendor.demo','/extension',{contributes:{xsmp:[{descriptor:'good',handler:'worker.wasm',apiVersion:'^2.0.0'}]}},'wasi',assert.fail);
 assert.equal(entries.length,1);assert.equal(entries[0].handlerPath,'/extension/worker.wasm');
});
test('a WASI server gets each path of a Windows extension as it reads it, a native one as Windows spells it',()=> {
 const windows=createTypeScriptLoader({'node:path':path.win32})(src('contributions/discovery.ts'));
 const manifest={contributes:{xsmp:[{descriptor:'tas.xsmpprofile',handler:'server\\tas.wasm',apiVersion:'^2.0.0',builtins:['models']}]}};
 const [wasi]=windows.resolveContributionEntries('vendor.demo','C:\\Users\\me\\ext',manifest,'wasi',assert.fail);
 assert.deepEqual([wasi.extensionRoot,wasi.descriptorPath,wasi.handlerPath,...wasi.builtins],['/c:/Users/me/ext','/c:/Users/me/ext/tas.xsmpprofile','/c:/Users/me/ext/server/tas.wasm','/c:/Users/me/ext/models']);
 const [native]=windows.resolveContributionEntries('vendor.demo','C:\\Users\\me\\ext',manifest,'native',assert.fail);
 assert.deepEqual([native.extensionRoot,native.handlerPath],['C:\\Users\\me\\ext','C:\\Users\\me\\ext\\server\\tas.wasm']);
});
test('malformed contribution manifests are isolated without losing valid sibling extensions',()=> {
 const messages=[];
 const manifest={contributes:{xsmp:[null,42,{descriptor:'bad',handler:'bad.js',apiVersion:'1',aliases:42},{descriptor:'bad-builtins',handler:'bad.js',apiVersion:'1',builtins:[null]},{descriptor:'',handler:'bad.js',apiVersion:'1'},{descriptor:'bad-handler',handler:'',apiVersion:'1'},{descriptor:'bad-wizard',handler:'bad.js',apiVersion:'1',wizard:{defaultSelected:'yes'}},{descriptor:'good',handler:'good.mjs',apiVersion:'^2'}]}};
 const entries=discovery.resolveContributionEntries('vendor.demo','/extension',manifest,'native',message=>messages.push(message));
 assert.equal(entries.length,1);assert.equal(entries[0].handlerPath,'/extension/good.mjs');
 assert.deepEqual(messages,[
  'vendor.demo: skipped invalid contribution at contributes.xsmp[0]: expected an object.',
  'vendor.demo: skipped invalid contribution at contributes.xsmp[1]: expected an object.',
  'vendor.demo: skipped invalid contribution at contributes.xsmp[2]: aliases must be an array of strings.',
  'vendor.demo: skipped invalid contribution at contributes.xsmp[3]: builtins must be an array of strings.',
  'vendor.demo: skipped invalid contribution at contributes.xsmp[4]: descriptor must be a non-empty string.',
  'vendor.demo: skipped invalid contribution at contributes.xsmp[5]: handler must be a non-empty string.',
  'vendor.demo: skipped invalid contribution at contributes.xsmp[6]: wizard.defaultSelected must be a boolean.']);
 const ignored=[];
 assert.deepEqual(plain(discovery.resolveContributionEntries('vendor.demo','/extension',{contributes:{xsmp:{}}},'native',message=>ignored.push(message))),[]);
 assert.deepEqual(ignored,['vendor.demo: ignored contributes.xsmp: expected an array.']);
});

function temporary(t){const directory=fs.mkdtempSync(path.join(os.tmpdir(),'xsmp-ui-'));t.after(()=>fs.rmSync(directory,{recursive:true,force:true}));return directory;}
test('wizard uses server summaries and all prompt types, supports no profile, and transmits values',async t=> {
 const parent=temporary(t), requests=[];let pick=0;const inputs=['Demo','custom'];const opened=[];
 const tool={id:'custom-tool',kind:'tool',label:'Custom',defaultSelected:true,extensionId:'vendor.custom',hasScaffolder:true};
 const vscode={Uri:URI,window:{showOpenDialog:async()=>[URI.file(parent)],showInputBox:async()=>inputs.shift(),showQuickPick:async(items,options)=>{
   switch(pick++){case 0:return undefined;case 1:assert.equal(items[0].picked,true);return[items[0]];case 2:return items[0];case 3:return items[1];case 4:return 'No';default:assert.fail('Unexpected prompt');}
 },showWarningMessage:async()=>undefined,showErrorMessage:async message=>assert.fail(message),showTextDocument:async document=>opened.push(document)},workspace:{workspaceFolders:[],openTextDocument:async file=>({file}),updateWorkspaceFolders:()=>assert.fail('No was selected')}};
 const wizard=createTypeScriptLoader({vscode})(src('wizard/wizard.ts'));
 const client={sendRequest:async(method,params)=>{requests.push({method,params});if(method==='xsmp/getContributionSummaries')return params==='profile'?[{...tool,id:'runtime',kind:'profile'}]:[tool];if(method==='xsmp/getContributionWizardPrompts')return[{id:'enabled',key:'tool.custom-tool.enabled',contributionId:'custom-tool',type:'boolean',label:'Enabled'},{id:'mode',key:'tool.custom-tool.mode',contributionId:'custom-tool',type:'choice',label:'Mode',choices:[{value:'one'},{value:'two'}]},{id:'name',key:'tool.custom-tool.name',contributionId:'custom-tool',type:'string',label:'Name'}];if(method==='xsmp/createProject')return{dependencies:[],failures:[]};assert.fail(method);}};
 await wizard.createProjectWizard(client);
 const scaffold=requests.find(item=>item.method==='xsmp/createProject').params;assert.equal(scaffold.selectedProfileId,undefined);assert.deepEqual(plain(scaffold.promptValues),{'tool.custom-tool.enabled':true,'tool.custom-tool.mode':'two','tool.custom-tool.name':'custom'});assert.deepEqual(plain(scaffold.selectedToolIds),['custom-tool']);assert.equal(opened.length,1);
 assert.equal(scaffold.projectDir,pathToFileURL(path.join(parent,'Demo')).toString());
});
// A project wizard that stops after the steps under test, with the given folders and answers.
function projectWizard(parent,{folders=[],summaries=[],answer}={}){
 const events=[],pickedItems=[],boxes=[];
 const vscode={Uri:URI,window:{showOpenDialog:async()=>[URI.file(parent)],showInputBox:async options=>{boxes.push(options);return 'Demo';},
  showQuickPick:async(items,options)=>{pickedItems.push({items,options});return options.placeHolder==='Add project to workspace?'?answer:options.canPickMany?[]:items[0];},
  showErrorMessage:async message=>assert.fail(message),showTextDocument:async()=>{events.push('catalogue');}},
  workspace:{workspaceFolders:folders,openTextDocument:async file=>({file}),updateWorkspaceFolders:(start,_count,folder)=>{events.push(`add ${start} ${folder.uri.fsPath}`);}}};
 const wizard=createTypeScriptLoader({vscode})(src('wizard/wizard.ts'));
 const client={sendRequest:async(method,params)=>method==='xsmp/getContributionSummaries'?(params==='profile'?summaries:[]):method==='xsmp/getContributionWizardPrompts'?[]:{}};
 return{run:()=>wizard.createProjectWizard(client),events,pickedItems,boxes};
}
test('the wizard refuses an invalid project name in its input box',async t=> {
 const harness=projectWizard(temporary(t));await harness.run();
 const {validateInput}=harness.boxes[0];
 for(const name of ['','1bad','a b','a/b'])assert.match(validateInput(name),/Project name must follow/);
 for(const name of ['a','Demo-1.x_y'])assert.equal(validateInput(name),undefined);
});
test('Enter in the profile list takes the default profile',async t=> {
 const profile=(id,label,defaultSelected)=>({id,kind:'profile',label,defaultSelected,extensionId:'',hasScaffolder:true});
 const harness=projectWizard(temporary(t),{summaries:[profile('esa-cdk','ESA-CDK',false),profile('b','B',false),profile('xsmp-sdk','XSMP-SDK',true)]});await harness.run();
 assert.deepEqual(harness.pickedItems[0].items.map(item=>item.label),['XSMP-SDK','B','ESA-CDK']);
});
test('a project already in the workspace is not offered to it',async t=> {
 const parent=temporary(t),harness=projectWizard(parent,{folders:[{uri:URI.file(parent)}]});await harness.run();
 assert.deepEqual(harness.events,['catalogue']);assert(!harness.pickedItems.some(pick=>pick.options.placeHolder==='Add project to workspace?'));
});
test('the catalogue opens before the first workspace folder restarts the extension host',async t=> {
 const parent=temporary(t),harness=projectWizard(parent,{answer:'Yes'});await harness.run();
 assert.deepEqual(harness.events,['catalogue',`add 0 ${path.join(parent,'Demo')}`]);
});
test('the file wizard refuses an invalid file name in its input box',async t=> {
 const directory=temporary(t);let box;
 const vscode={Uri:URI,window:{showInputBox:async options=>{box=options;return 'Good';},showErrorMessage:async message=>assert.fail(message),showTextDocument:async()=>{}},workspace:{openTextDocument:async file=>({file})}};
 await createTypeScriptLoader({vscode})(src('wizard/file-wizard.ts')).createXsmpStarterFileWizard('catalogue',URI.file(directory));
 assert(fs.existsSync(path.join(directory,'Good.xsmpcat')));
 assert.match(box.validateInput('1bad'),/File name must follow/);assert.equal(box.validateInput('Good'),undefined);
});
test('extension manifest: commands, entry point, host kind and workspace trust',()=> {
 const metadata=JSON.parse(fs.readFileSync(new URL('../package.json',import.meta.url),'utf8'));
 const required=['xsmp.wizard','xsmp.newFile','xsmp.newCatalogue','xsmp.newConfiguration','xsmp.newAssembly','xsmp.newLinkBase','xsmp.newSchedule','xsmp.generate','xsmp.generateAll','xsmp.importSmp','xsmp.openSmpMirror','xsmp.openSmpXmlSource','xsmp.openDocumentation'];
 assert.deepEqual(metadata.contributes.commands.map(command=>command.command).sort(),required.sort());
 assert.equal(metadata.browser,undefined);assert.equal(metadata.main,'./out/extension.js');
 assert.deepEqual(metadata.extensionKind,['workspace']);
 assert.equal(metadata.capabilities.untrustedWorkspaces.supported,false);
 assert(metadata.activationEvents.includes('workspaceContains:**/xsmp.project'));
});
test('the TAS-MDK package script runs vsce through node, which every shell can start',()=> {
 const root=new URL('../../extensions/tas-mdk/',import.meta.url);
 const [command,script]=JSON.parse(fs.readFileSync(new URL('package.json',root),'utf8')).scripts.package.split(' ');
 assert.equal(command,'node');assert(fs.existsSync(new URL(script,root)));
});

test('a registration report shows isolated failures',async()=> {
 const messages=[], logs=[];
 const vscode={window:{showWarningMessage:async message=>{messages.push(message);return'Show Details';}}};
 const register=createTypeScriptLoader({vscode})(src('contributions/register.ts'));
 const channel={appendLine:line=>logs.push(line),show:()=>logs.push('show')};
 const report={registered:[{id:'later',kind:'tool',extensionId:'later.extension'}],failures:[{extensionId:'broken',phase:'validation',message:'Invalid registrar',descriptorPath:'/broken/descriptor',handlerPath:'/broken/handler'}]};
 register.reportContributionRegistration(report,channel);
 await new Promise(resolve=>setImmediate(resolve));
 assert(messages.every(message=>message.includes('validation')));assert(logs.some(line=>line.includes('descriptor:')));assert(logs.includes('show'));
});

function workflowModule(vscode){return createTypeScriptLoader({vscode})(src('workflows.ts'));}
test('generation on save follows the active document and its language',()=> {
 const active={uri:URI.file('/workspace/model.xsmpcat'),languageId:'xsmpcat',isUntitled:false};let enabled=true;
 const vscode={Uri:URI,window:{activeTextEditor:{document:active}},workspace:{getConfiguration:()=>({get:(_key,fallback)=>enabled??fallback})}};
 const workflow=workflowModule(vscode);
 assert.equal(workflow.shouldGenerateOnSave(active),true);
 for(const changed of [{...active,isUntitled:true},{...active,uri:VscodeURI.parse('xsmp:/builtin.xsmpcat')},{...active,uri:URI.file('/workspace/background.xsmpcat')},{...active,languageId:'xsmp-unrelated'}])assert.equal(workflow.shouldGenerateOnSave(changed),false);
 enabled=false;assert.equal(workflow.shouldGenerateOnSave(active),false);
});
test('SMP import and XML-source commands resolve every virtual mirror to its XML file',()=> {
 const workflow=workflowModule({Uri:URI});
 for(const [xml,native]of[['smpcat','xsmpcat'],['smpcfg','xsmpcfg'],['smpasb','xsmpasb'],['smplnk','xsmplnk'],['smpsed','xsmpsed']]){
  assert.equal(workflow.resolveImportSmpTarget(VscodeURI.parse(`xsmp-smp:/workspace/model.${native}`)).fsPath,`/workspace/model.${xml}`);
  assert.equal(workflow.resolveImportSmpTarget(VscodeURI.file(`/workspace/model.${xml}`)).fsPath,`/workspace/model.${xml}`);
 }
 assert.equal(workflow.resolveImportSmpTarget(VscodeURI.file('/workspace/model.xsmpcat')),undefined);
 assert.equal(workflow.resolveImportSmpTarget(VscodeURI.parse('xsmp:/builtin.xsmpcat')),undefined);
});
test('generation reports keep the singular, empty and skipped project details',()=> {
 const info=[],warnings=[];const workflow=workflowModule({window:{showInformationMessage:message=>info.push(message),showWarningMessage:message=>warnings.push(message)}});
 workflow.reportProjectGeneration({generatedProjects:['Mission'],skippedProjects:[]},'Done.');
 workflow.reportProjectGeneration({generatedProjects:[],skippedProjects:[]},'Done.');
 workflow.reportProjectGeneration({generatedProjects:['Mission'],skippedProjects:[{projectName:'Broken',errorCount:1}]},'Done.');
 assert.deepEqual(info,["Done. Generated project 'Mission'.",'Done. No project was generated.']);
 assert.deepEqual(warnings,['Generated 1 project(s). Skipped 1 project(s): Broken (1 error)']);
});

test('native and WASI launchers pass the runner and host module of server/ and honor extension-host paths',t=> {
 const directory=temporary(t);fs.mkdirSync(path.join(directory,'server'));fs.writeFileSync(path.join(directory,'server/wasi-run.mjs'),'');fs.writeFileSync(path.join(directory,'server/wasi-host.mjs'),'');
 const native=path.join(directory,'native-server'),wasm=path.join(directory,'custom-no-suffix');fs.writeFileSync(native,Buffer.from([0x7f,0x45,0x4c,0x46]));fs.writeFileSync(wasm,Buffer.from([0,0x61,0x73,0x6d]));
 const vscode={workspace:{getConfiguration:()=>({get:()=>undefined})}};
 const extension=createTypeScriptLoader({vscode,'vscode-languageclient/node':{State:{},LanguageClient:class{}}})(src('extension.ts'));
 const context={extensionPath:directory};
 const nativeOptions=extension.createServerOptions(native,context),wasiOptions=extension.createServerOptions(wasm,context);
 assert.equal(nativeOptions.command,native);assert.equal(wasiOptions.command,process.execPath);assert.deepEqual(plain(wasiOptions.args),['--no-warnings',path.join(directory,'server/wasi-run.mjs'),'--host-module',path.join(directory,'server/wasi-host.mjs'),wasm]);
 assert.equal(wasiOptions.options.env.ELECTRON_RUN_AS_NODE,'1');assert.equal(nativeOptions.options.env.ELECTRON_RUN_AS_NODE,process.env.ELECTRON_RUN_AS_NODE);
 const withLibraries=extension.createServerOptions(wasm,context,['/a/one.wasm','/b/two.wasm']);assert.equal(withLibraries.options.env.XSMP_EXTENSION_LIBRARIES,'/a/one.wasm\n/b/two.wasm');
 assert.equal(extension.isWasmBinary(wasm),true);assert.equal(extension.isWasmBinary(native),false);
 // A WebAssembly server cannot start without the runner; a native one can.
 fs.unlinkSync(path.join(directory,'server/wasi-run.mjs'));
 assert.throws(()=>extension.createServerOptions(wasm,context),/WASI runner/);
 assert.equal(extension.createServerOptions(native,context).command,native);
});

test('F5 selects the active Cursiv preset before bundled runtimes and preserves an explicit override',t=> {
 const directory=temporary(t),preset=path.join(directory,'active preset'),extensionDir=path.join(directory,'vscode');
 fs.mkdirSync(preset);fs.mkdirSync(path.join(extensionDir,'server'),{recursive:true});
 const active=path.join(preset,'xsmp_lsp'),bundled=path.join(extensionDir,'server/xsmp_lsp.wasm');
 fs.writeFileSync(active,Buffer.from([0,0x61,0x73,0x6d]));fs.writeFileSync(bundled,Buffer.from([0,0x61,0x73,0x6d]));
 const previous=process.env.CURSIV_SERVER_DIR;process.env.CURSIV_SERVER_DIR=` ${preset} `;
 t.after(()=>{if(previous===undefined)delete process.env.CURSIV_SERVER_DIR;else process.env.CURSIV_SERVER_DIR=previous;});
 let configured;const vscode={workspace:{getConfiguration:()=>({get:()=>configured})}};
 const extension=createTypeScriptLoader({vscode,'vscode-languageclient/node':{State:{},LanguageClient:class{}}})(src('extension.ts'));
 assert.equal(extension.resolveServerCommand({extensionPath:extensionDir}),active);
 configured='/explicit/server';assert.equal(extension.resolveServerCommand({extensionPath:extensionDir}),configured);
 configured=undefined;fs.unlinkSync(active);assert.equal(extension.resolveServerCommand({extensionPath:extensionDir}),bundled);
 // The WebAssembly server wins over a packaged native one, which is used without it.
 const packaged=path.join(extensionDir,'bin',process.platform==='win32'?'xsmp_lsp.exe':'xsmp_lsp');
 fs.mkdirSync(path.dirname(packaged));fs.writeFileSync(packaged,'');
 assert.equal(extension.resolveServerCommand({extensionPath:extensionDir}),bundled);
 fs.unlinkSync(bundled);assert.equal(extension.resolveServerCommand({extensionPath:extensionDir}),packaged);
 fs.unlinkSync(packaged);assert.equal(extension.resolveServerCommand({extensionPath:extensionDir}),undefined);
});

function workflowHarness({startup=false,folders=[],answer}={}){
 const commands=new Map(),requests=[],info=[],warnings=[],errors=[],logs=[],progress=[];let saved;
 class Emitter{event=()=>({dispose(){}});fire(){}dispose(){}}
 class FileError extends Error{constructor(code){super(code);this.code=code;}static NoPermissions(){return new FileError('NoPermissions');}}
 const doc={uri:VscodeURI.file('/workspace/Demo.xsmpcat'),languageId:'xsmpcat',isUntitled:false};
 const vscode={Uri:VscodeURI,EventEmitter:Emitter,FileSystemError:FileError,FileType:{File:1,Unknown:0},FileChangeType:{Changed:2,Deleted:3},ProgressLocation:{Notification:15},CancellationError:class extends Error{},
 commands:{registerCommand:(id,callback)=>{commands.set(id,callback);return{dispose(){}};}},
 window:{activeTextEditor:{document:doc},withProgress:async(options,task)=>{progress.push(options);return task({report(){}},{isCancellationRequested:false});},showInformationMessage:message=>info.push(message),showWarningMessage:async message=>{warnings.push(message);return'Overwrite';},showErrorMessage:message=>errors.push(message),showTextDocument:async()=>{}},
 workspace:{workspaceFolders:folders,getConfiguration:()=>({get:(key,fallback)=>key==='generateAllOnStartup'?startup:fallback}),onDidSaveTextDocument:callback=>{saved=callback;return{dispose(){}};},registerFileSystemProvider:()=>({dispose(){}}),fs:{stat:async()=>{throw new FileError('FileNotFound');}},openTextDocument:async uri=>({uri})}};
 const client={onNotification:()=>({dispose(){}}),sendRequest:async(method,params,token)=>{requests.push({method,params,token});if(answer)return answer();return{generatedProjects:['Demo'],skippedProjects:[],failures:[],outputPath:'/workspace/Demo.xsmpcat',warnings:[]};}};
 const workflow=workflowModule(vscode);workflow.registerWorkflows({subscriptions:[]},()=>client,{appendLine:line=>logs.push(line)});
 return{commands,requests,info,warnings,errors,logs,progress,doc,save:()=>saved(doc),vscode};
}
test('a generation runs under a notification that cancels it without a report',async()=>{
 const harness=workflowHarness();
 await harness.commands.get('xsmp.generateAll')();
 assert.deepEqual(harness.progress.map(options=>[options.title,options.cancellable]),[['Generating XSMP sources',true]]);
 assert(harness.requests[0].token);assert.equal(harness.info.length,1);
 const cancelled=workflowHarness({answer:()=>{throw Object.assign(new Error('cancelled'),{code:-32800});}});
 await cancelled.commands.get('xsmp.generate')();
 assert.equal(cancelled.requests.length,1);assert.deepEqual([cancelled.info,cancelled.errors],[[],[]]);
});
test('automatic save generation stays silent on success and startup waits for a workspace',async()=>{
 const harness=workflowHarness({startup:true});assert.equal(harness.requests.length,0);
 await harness.save();assert.equal(harness.requests[0].method,'xsmp/generateProject');assert.equal(harness.info.length,0);assert(harness.logs.some(message=>message.includes("Generated project 'Demo' after saving")));
 const before=harness.requests.length;harness.vscode.window.activeTextEditor={document:{...harness.doc,uri:VscodeURI.file('/workspace/Other.xsmpcat')}};await harness.save();assert.equal(harness.requests.length,before);
});
test('SMP import from a virtual editor uses the XML file URI and asks before overwriting',async()=>{
 const harness=workflowHarness();harness.vscode.window.activeTextEditor.document={uri:VscodeURI.parse('xsmp-smp:/workspace/Demo.xsmpcat')};
 await harness.commands.get('xsmp.importSmp')();
 const request=harness.requests.find(request=>request.method==='xsmp/importSmpFile');
 assert.equal(request.params.uri,'file:///workspace/Demo.smpcat');assert.equal(request.params.outputUri,'file:///workspace/Demo.xsmpcat');assert.equal(request.params.force,false);
});
