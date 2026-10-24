import * as vscode from 'vscode';
import { exec } from 'child_process';
import * as path from 'path';

export function activate(context: vscode.ExtensionContext) {
    // Эта строчка ОБЯЗАТЕЛЬНО должна появиться в Debug Console при старте хоста
    console.log('Расширение "Code Style Critic" успешно запущено и следит за стилем!');

    const saveEvent = vscode.workspace.onDidSaveTextDocument((document: vscode.TextDocument) => {
        const filePath = document.fileName;
        
        const isWindows = process.platform === 'win32';
        const binaryName = isWindows ? 'main.exe' : 'main';
        
        const analyzerPath = path.join(context.extensionPath, binaryName);
        const command = `"${analyzerPath}" "${filePath}"`;

        console.log(`Попытка запустить команду: ${command}`);

        exec(command, (error: Error | null, stdout: string, stderr: string) => {
            console.log(`C++ stdout: "${stdout}"`);
            console.log(`C++ stderr: "${stderr}"`);

            if (error) {
                console.error(`Ошибка exec: ${error.message}`);
                vscode.window.showErrorMessage(`Ошибка запуска бэкенда: ${error.message}`);
                return;
            }
        
            const verdict = stdout.trim();
            if (!verdict) { 
                console.log("Вердикт пустой, уведомление не вызывается.");
                return; 
            }

            if (verdict.includes('завершена')) {
                vscode.window.showInformationMessage(`CodeCritic: ${verdict}`);
            } else {
                vscode.window.showWarningMessage(`CodeCritic: ${verdict}`);
            }
        });
    });

    context.subscriptions.push(saveEvent);
}

export function deactivate() {
    console.log('Расширение "CodeCritic" выключено.');
}
