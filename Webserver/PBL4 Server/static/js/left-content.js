function toggleSubMenu() {
    const subMenu = document.getElementById('sub-menu');
    subMenu.classList.toggle('active'); // Thêm hoặc xóa class active
}

// Đóng menu con khi nhấn ra ngoài
window.onclick = function(event) {
    const subMenu = document.getElementById('sub-menu');
    const targetElement = event.target.closest('.navbar li'); // Kiểm tra nếu nhấn vào menu
    if (!targetElement) {
        subMenu.classList.remove('active'); // Ẩn menu con khi nhấn ra ngoài
    }
}
document.querySelectorAll('.sub-menu li a').forEach(item => {
    item.addEventListener('click', function() {
        // Xóa lớp active khỏi tất cả các mục
        document.querySelectorAll('.sub-menu li a').forEach(link => {
            link.classList.remove('active');
        });
        // Thêm lớp active cho mục đang được chọn
        this.classList.add('active');
    });
});