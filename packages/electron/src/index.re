open BsElectron;

[@bs.val] external dirname : string = "__dirname";

[%bs.raw "require('electron-debug')({showDevTools: true})"];

let winRef: ref(option(BrowserWindow.t)) = ref(None);

let createWindow = () => {
    ElectronUpdater.checkForUpdatesAndNotify ();

    let win = BrowserWindow.make();
    winRef := Some(win);

    BrowserWindow.loadUrl("http://www.mopho.local/", win);
    BrowserWindow.webContents(win)
        |> WebContents.openDevTools;

    BrowserWindow.onClosed(() => {
        winRef := None;
        App.quit();
    }, win);
};

App.onReady(createWindow);

App.onActivateMac(() =>
    switch winRef^ {
        | None => createWindow()
        | Some(_) => ()
    }
);
