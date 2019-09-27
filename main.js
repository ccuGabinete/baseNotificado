const { app, BrowserWindow } = require('electron');

app.on('ready', () => {
    console.log('app run');

    let mainWindow = new BrowserWindow({
        width: 600,
        heigth: 400
    })

    mainWindow.loadURL(__dirname + '/src/app/index.html');

})

app.on('window-all-closed', () => {
    app.quit();
})

app.on('teste', (texto) => {
    console.log(texto);
    console.log('OK');
})
