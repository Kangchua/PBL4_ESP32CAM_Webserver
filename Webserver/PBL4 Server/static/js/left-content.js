let updateInterval;

function toggleSubMenu() {
    const subMenu = document.getElementById('sub-menu');
    subMenu.classList.toggle('active');
}

window.onclick = function (event) {
    const subMenu = document.getElementById('sub-menu');
    const targetElement = event.target.closest('.navbar li');
    if (!targetElement) {
        subMenu.classList.remove('active');
    }
};

document.querySelectorAll('.sub-menu li a').forEach(item => {
    item.addEventListener('click', function () {
        document.querySelectorAll('.sub-menu li a').forEach(link => {
            link.classList.remove('active');
        });
        this.classList.add('active');
    });
});

// Function to set the update mode
function setMode(mode) {
    clearInterval(updateInterval); // Clear any existing intervals

    if (mode === 'direct') {
        // "Trực Tiếp" mode updates every 5 seconds
        updateInterval = setInterval(() => {
            document.querySelector('iframe[name="sidebar"]').contentWindow.updateIdentification();
        }, 5000);
    } else if (mode === 'review') {
        // "Xem Lại" mode updates the list every 5 seconds
        updateInterval = setInterval(() => {
            document.querySelector('iframe[name="sidebar"]').contentWindow.fetchStorageData();
        }, 5000);
    } else {
        // Stop updates for other modes
        clearInterval(updateInterval);
    }
}
