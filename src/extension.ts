import * as vscode from 'vscode';
import { exec } from 'child_process';
import * as path from 'path';

export function activate(context: vscode.ExtensionContext) {
    console.log('Расширение "Code Style Critic" успешно запущено и следит за стилем!');

    const saveEvent = vscode.workspace.onDidSaveTextDocument((document: vscode.TextDocument) => {
        const filePath = document.fileName;
        
        const fileExtension = path.extname(filePath).toLowerCase();
        if (fileExtension !== '.cpp' && fileExtension !== '.c') {
            console.log(`[CodeCritic] Пропуск: файл ${fileExtension} не относится к C/C++.`);
            return;
        }
        
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
                vscode.window.showInformationMessage(`CodeCritic: ${verdict}`, { modal: true });
            } else {
                vscode.window.showWarningMessage(`CodeCritic: ${verdict}`, { modal: true });
            }
        });
    });

    context.subscriptions.push(saveEvent);
}

export function deactivate() {
    console.log('Расширение "CodeCritic" выключено.');
}
