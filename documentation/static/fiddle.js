window.addEventListener('message', function postMessageHandler(e) {
    const fiddleEl = document.getElementById(e.data.embedID);
    if (fiddleEl && e.data.event === 'resize' && fiddleEl.offsetHeight < e.data.contentHeight) {
        fiddleEl.style.height = e.data.contentHeight + 'px';
    }
});
