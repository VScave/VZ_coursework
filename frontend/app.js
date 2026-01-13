let sessionId = localStorage.getItem('session_id');
let userRole = localStorage.getItem('role');
let students = [];
let filteredStudents = [];
let grades = [];
let currentSort = null;

// Проверка авторизации при загрузке
window.addEventListener('DOMContentLoaded', () => {
    if (sessionId) {
        document.getElementById('login-required').style.display = 'none';
        document.getElementById('main-content').style.display = 'block';
        document.getElementById('user-info').style.display = 'flex';
        
        if (userRole === 'admin') {
            document.getElementById('admin-btn').style.display = 'block';
        }
        
        loadStudents();
    } else {
        document.getElementById('login-required').style.display = 'block';
        document.getElementById('main-content').style.display = 'none';
    }
});

function logout() {
    localStorage.removeItem('session_id');
    localStorage.removeItem('role');
    window.location.reload();
}

async function loadStudents() {
    try {
        const response = await fetch(`/api/students?session_id=${sessionId}`);
        const data = await response.json();
        
        if (response.status === 401) {
            logout();
            return;
        }
        
        students = data;
        filteredStudents = [...students];
        populateGroupFilter();
        // Обновить фильтр админа, если он открыт
        if (document.getElementById('admin-panel').style.display !== 'none') {
            populateAdminGroupFilter();
            filteredAdminStudents = [...students];
        }
        displayStudents();
        updateStudentSelect();
    } catch (error) {
        console.error('Ошибка загрузки студентов:', error);
    }
}

function populateGroupFilter() {
    const filter = document.getElementById('group-filter');
    const groups = [...new Set(students.map(s => s.group_name))].sort();
    
    // Очистить существующие опции (кроме "Все группы")
    filter.innerHTML = '<option value="">Все группы</option>';
    
    groups.forEach(group => {
        const option = document.createElement('option');
        option.value = group;
        option.textContent = group;
        filter.appendChild(option);
    });
}

function filterByGroup() {
    const selectedGroup = document.getElementById('group-filter').value;
    
    if (selectedGroup === '') {
        filteredStudents = [...students];
    } else {
        filteredStudents = students.filter(s => s.group_name === selectedGroup);
    }
    
    // Применить текущую сортировку, если она есть
    if (currentSort) {
        sortStudents(currentSort, false);
    } else {
        displayStudents();
    }
}

function sortStudents(sortType, updateDisplay = true) {
    currentSort = sortType;
    
    if (sortType === 'name') {
        filteredStudents.sort((a, b) => {
            const nameA = `${a.surname} ${a.name}`.toLowerCase();
            const nameB = `${b.surname} ${b.name}`.toLowerCase();
            return nameA.localeCompare(nameB);
        });
    } else if (sortType === 'group') {
        filteredStudents.sort((a, b) => {
            if (a.group_name !== b.group_name) {
                return a.group_name.localeCompare(b.group_name);
            }
            // Если группы одинаковые, сортируем по имени
            const nameA = `${a.surname} ${a.name}`.toLowerCase();
            const nameB = `${b.surname} ${b.name}`.toLowerCase();
            return nameA.localeCompare(nameB);
        });
    }
    
    if (updateDisplay) {
        displayStudents();
    }
}

function displayStudents() {
    const list = document.getElementById('students-list');
    list.innerHTML = '';
    
    if (filteredStudents.length === 0) {
        list.innerHTML = '<p style="text-align: center; color: #666; padding: 20px;">Студенты не найдены</p>';
        return;
    }
    
    // Группировка по группам для лучшего отображения
    const grouped = {};
    filteredStudents.forEach(student => {
        if (!grouped[student.group_name]) {
            grouped[student.group_name] = [];
        }
        grouped[student.group_name].push(student);
    });
    
    // Отображение по группам
    Object.keys(grouped).sort().forEach(groupName => {
        const groupDiv = document.createElement('div');
        groupDiv.className = 'group-section';
        groupDiv.innerHTML = `<h3 class="group-header">Группа: ${groupName}</h3>`;
        
        const studentsContainer = document.createElement('div');
        studentsContainer.className = 'students-group';
        
        grouped[groupName].forEach(student => {
            const card = document.createElement('div');
            card.className = 'student-card';
            card.innerHTML = `
                <h4>${student.name} ${student.surname}</h4>
                <p>Группа: ${student.group_name}</p>
            `;
            studentsContainer.appendChild(card);
        });
        
        groupDiv.appendChild(studentsContainer);
        list.appendChild(groupDiv);
    });
}

function updateStudentSelect() {
    const select = document.getElementById('student-select');
    select.innerHTML = '<option value="">-- Выберите --</option>';
    students.forEach(student => {
        const option = document.createElement('option');
        option.value = student.id;
        option.textContent = `${student.name} ${student.surname} (${student.group_name})`;
        select.appendChild(option);
    });
}

async function getPrediction() {
    const studentId = document.getElementById('student-select').value;
    if (!studentId) {
        alert('Выберите студента');
        return;
    }
    
    try {
        const response = await fetch(`/api/predict?session_id=${sessionId}&student_id=${studentId}`);
        const data = await response.json();
        
        if (response.status === 401) {
            logout();
            return;
        }
        
        document.getElementById('prediction-result').textContent = data.prediction || 'Ошибка получения прогноза';
    } catch (error) {
        console.error('Ошибка получения прогноза:', error);
        document.getElementById('prediction-result').textContent = 'Ошибка соединения';
    }
}

function showAdminPanel() {
    document.getElementById('admin-panel').style.display = 'block';
    loadAdminData();
}

let filteredAdminStudents = [];
let adminCurrentSort = null;

async function loadAdminData() {
    await loadStudents();
    await loadGrades();
    filteredAdminStudents = [...students];
    populateAdminGroupFilter();
    displayAdminStudents();
    displayAdminGrades();
}

function populateAdminGroupFilter() {
    const filter = document.getElementById('admin-group-filter');
    if (!filter) return;
    
    const groups = [...new Set(students.map(s => s.group_name))].sort();
    filter.innerHTML = '<option value="">Все группы</option>';
    
    groups.forEach(group => {
        const option = document.createElement('option');
        option.value = group;
        option.textContent = group;
        filter.appendChild(option);
    });
}

function filterAdminByGroup() {
    const selectedGroup = document.getElementById('admin-group-filter').value;
    
    if (selectedGroup === '') {
        filteredAdminStudents = [...students];
    } else {
        filteredAdminStudents = students.filter(s => s.group_name === selectedGroup);
    }
    
    if (adminCurrentSort) {
        sortAdminStudents(adminCurrentSort, false);
    } else {
        displayAdminStudents();
    }
}

function sortAdminStudents(sortType, updateDisplay = true) {
    adminCurrentSort = sortType;
    
    if (sortType === 'name') {
        filteredAdminStudents.sort((a, b) => {
            const nameA = `${a.surname} ${a.name}`.toLowerCase();
            const nameB = `${b.surname} ${b.name}`.toLowerCase();
            return nameA.localeCompare(nameB);
        });
    } else if (sortType === 'group') {
        filteredAdminStudents.sort((a, b) => {
            if (a.group_name !== b.group_name) {
                return a.group_name.localeCompare(b.group_name);
            }
            const nameA = `${a.surname} ${a.name}`.toLowerCase();
            const nameB = `${b.surname} ${b.name}`.toLowerCase();
            return nameA.localeCompare(nameB);
        });
    }
    
    if (updateDisplay) {
        displayAdminStudents();
    }
}

async function loadGrades() {
    try {
        const response = await fetch(`/api/grades?session_id=${sessionId}`);
        const data = await response.json();
        
        if (response.status === 401) {
            logout();
            return;
        }
        
        grades = data;
    } catch (error) {
        console.error('Ошибка загрузки оценок:', error);
    }
}

function displayAdminStudents() {
    const list = document.getElementById('students-admin-list');
    list.innerHTML = '';
    
    if (filteredAdminStudents.length === 0) {
        list.innerHTML = '<p style="text-align: center; color: #666; padding: 20px;">Студенты не найдены</p>';
        return;
    }
    
    // Группировка по группам для лучшего отображения
    const grouped = {};
    filteredAdminStudents.forEach(student => {
        if (!grouped[student.group_name]) {
            grouped[student.group_name] = [];
        }
        grouped[student.group_name].push(student);
    });
    
    // Отображение по группам
    Object.keys(grouped).sort().forEach(groupName => {
        const groupDiv = document.createElement('div');
        groupDiv.className = 'group-section';
        groupDiv.innerHTML = `<h3 class="group-header">Группа: ${groupName}</h3>`;
        
        const studentsContainer = document.createElement('div');
        studentsContainer.className = 'students-group';
        
        grouped[groupName].forEach(student => {
            const card = document.createElement('div');
            card.className = 'student-card';
            card.innerHTML = `
                <h4>${student.name} ${student.surname}</h4>
                <p>Группа: ${student.group_name}</p>
                <div style="margin-top: 10px;">
                    <button class="edit-btn" onclick="editStudent(${student.id})">Редактировать</button>
                    <button class="delete-btn" onclick="deleteStudent(${student.id})">Удалить</button>
                </div>
            `;
            studentsContainer.appendChild(card);
        });
        
        groupDiv.appendChild(studentsContainer);
        list.appendChild(groupDiv);
    });
}

function displayAdminGrades() {
    const list = document.getElementById('grades-admin-list');
    list.innerHTML = '';
    
    grades.forEach(grade => {
        const student = students.find(s => s.id === grade.student_id);
        const card = document.createElement('div');
        card.className = 'grade-card';
        card.innerHTML = `
            <h4>${student ? student.name + ' ' + student.surname : 'Неизвестно'}</h4>
            <p>Предмет: ${grade.subject}</p>
            <p>Оценка: ${grade.grade}, Семестр: ${grade.semester}</p>
            <p>Посещаемость: ${grade.attendance_percent}%, Выполнение заданий: ${grade.assignment_completion}%</p>
            <p>Результат экзамена: ${grade.exam_result || 'Нет'}</p>
            <button class="edit-btn" onclick="editGrade(${grade.id})">Редактировать</button>
            <button class="delete-btn" onclick="deleteGrade(${grade.id})">Удалить</button>
        `;
        list.appendChild(card);
    });
}

function showAddStudentForm() {
    const modal = document.createElement('div');
    modal.className = 'form-modal';
    modal.id = 'add-student-modal';
    modal.innerHTML = `
        <div class="form-modal-content">
            <h3>Добавить студента</h3>
            <form id="add-student-form">
                <div>
                    <label>Имя:</label>
                    <input type="text" id="student-name" required>
                </div>
                <div>
                    <label>Фамилия:</label>
                    <input type="text" id="student-surname" required>
                </div>
                <div>
                    <label>Группа:</label>
                    <input type="text" id="student-group" required>
                </div>
                <div class="form-buttons">
                    <button type="submit">Добавить</button>
                    <button type="button" onclick="closeModal('add-student-modal')">Отмена</button>
                </div>
            </form>
        </div>
    `;
    document.body.appendChild(modal);
    modal.style.display = 'block';
    
    document.getElementById('add-student-form').addEventListener('submit', async (e) => {
        e.preventDefault();
        const formData = new URLSearchParams();
        formData.append('session_id', sessionId);
        formData.append('name', document.getElementById('student-name').value);
        formData.append('surname', document.getElementById('student-surname').value);
        formData.append('group_name', document.getElementById('student-group').value);
        
        try {
            const response = await fetch('/api/admin/students/add', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: formData
            });
            
            const data = await response.json();
            if (data.success) {
                closeModal('add-student-modal');
                loadAdminData();
            } else {
                alert('Ошибка добавления студента');
            }
        } catch (error) {
            alert('Ошибка соединения');
        }
    });
}

function showAddGradeForm() {
    const modal = document.createElement('div');
    modal.className = 'form-modal';
    modal.id = 'add-grade-modal';
    modal.innerHTML = `
        <div class="form-modal-content">
            <h3>Добавить оценку</h3>
            <form id="add-grade-form">
                <div>
                    <label>Студент:</label>
                    <select id="grade-student-id" required></select>
                </div>
                <div>
                    <label>Предмет:</label>
                    <input type="text" id="grade-subject" required>
                </div>
                <div>
                    <label>Оценка (1-5):</label>
                    <input type="number" id="grade-value" min="1" max="5" required>
                </div>
                <div>
                    <label>Семестр:</label>
                    <input type="number" id="grade-semester" required>
                </div>
                <div>
                    <label>Посещаемость (%):</label>
                    <input type="number" id="grade-attendance" min="0" max="100" step="0.1" required>
                </div>
                <div>
                    <label>Выполнение заданий (%):</label>
                    <input type="number" id="grade-assignment" min="0" max="100" step="0.1" required>
                </div>
                <div>
                    <label>Результат экзамена (1-5):</label>
                    <input type="number" id="grade-exam" min="1" max="5" required>
                </div>
                <div class="form-buttons">
                    <button type="submit">Добавить</button>
                    <button type="button" onclick="closeModal('add-grade-modal')">Отмена</button>
                </div>
            </form>
        </div>
    `;
    document.body.appendChild(modal);
    modal.style.display = 'block';
    
    // Заполнить селект студентов
    const select = document.getElementById('grade-student-id');
    students.forEach(student => {
        const option = document.createElement('option');
        option.value = student.id;
        option.textContent = `${student.name} ${student.surname}`;
        select.appendChild(option);
    });
    
    document.getElementById('add-grade-form').addEventListener('submit', async (e) => {
        e.preventDefault();
        const formData = new URLSearchParams();
        formData.append('session_id', sessionId);
        formData.append('student_id', document.getElementById('grade-student-id').value);
        formData.append('subject', document.getElementById('grade-subject').value);
        formData.append('grade', document.getElementById('grade-value').value);
        formData.append('semester', document.getElementById('grade-semester').value);
        formData.append('attendance', document.getElementById('grade-attendance').value);
        formData.append('assignment', document.getElementById('grade-assignment').value);
        formData.append('exam_result', document.getElementById('grade-exam').value);
        
        try {
            const response = await fetch('/api/admin/grades/add', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: formData
            });
            
            const data = await response.json();
            if (data.success) {
                closeModal('add-grade-modal');
                loadAdminData();
            } else {
                alert('Ошибка добавления оценки');
            }
        } catch (error) {
            alert('Ошибка соединения');
        }
    });
}

async function deleteStudent(id) {
    if (!confirm('Удалить студента?')) return;
    
    const formData = new URLSearchParams();
    formData.append('session_id', sessionId);
    formData.append('id', id);
    
    try {
        const response = await fetch('/api/admin/students/delete', {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body: formData
        });
        
        const data = await response.json();
        if (data.success) {
            loadAdminData();
        } else {
            alert('Ошибка удаления');
        }
    } catch (error) {
        alert('Ошибка соединения');
    }
}

async function deleteGrade(id) {
    if (!confirm('Удалить оценку?')) return;
    
    const formData = new URLSearchParams();
    formData.append('session_id', sessionId);
    formData.append('id', id);
    
    try {
        const response = await fetch('/api/admin/grades/delete', {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body: formData
        });
        
        const data = await response.json();
        if (data.success) {
            loadAdminData();
        } else {
            alert('Ошибка удаления');
        }
    } catch (error) {
        alert('Ошибка соединения');
    }
}

function editStudent(id) {
    const student = students.find(s => s.id === id);
    if (!student) return;
    
    const modal = document.createElement('div');
    modal.className = 'form-modal';
    modal.id = 'edit-student-modal';
    modal.innerHTML = `
        <div class="form-modal-content">
            <h3>Редактировать студента</h3>
            <form id="edit-student-form">
                <input type="hidden" id="edit-student-id" value="${student.id}">
                <div>
                    <label>Имя:</label>
                    <input type="text" id="edit-student-name" value="${student.name}" required>
                </div>
                <div>
                    <label>Фамилия:</label>
                    <input type="text" id="edit-student-surname" value="${student.surname}" required>
                </div>
                <div>
                    <label>Группа:</label>
                    <input type="text" id="edit-student-group" value="${student.group_name}" required>
                </div>
                <div class="form-buttons">
                    <button type="submit">Сохранить</button>
                    <button type="button" onclick="closeModal('edit-student-modal')">Отмена</button>
                </div>
            </form>
        </div>
    `;
    document.body.appendChild(modal);
    modal.style.display = 'block';
    
    document.getElementById('edit-student-form').addEventListener('submit', async (e) => {
        e.preventDefault();
        const formData = new URLSearchParams();
        formData.append('session_id', sessionId);
        formData.append('id', document.getElementById('edit-student-id').value);
        formData.append('name', document.getElementById('edit-student-name').value);
        formData.append('surname', document.getElementById('edit-student-surname').value);
        formData.append('group_name', document.getElementById('edit-student-group').value);
        
        try {
            const response = await fetch('/api/admin/students/update', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: formData
            });
            
            const data = await response.json();
            if (data.success) {
                closeModal('edit-student-modal');
                loadAdminData();
            } else {
                alert('Ошибка обновления');
            }
        } catch (error) {
            alert('Ошибка соединения');
        }
    });
}

function editGrade(id) {
    const grade = grades.find(g => g.id === id);
    if (!grade) return;
    
    const modal = document.createElement('div');
    modal.className = 'form-modal';
    modal.id = 'edit-grade-modal';
    modal.innerHTML = `
        <div class="form-modal-content">
            <h3>Редактировать оценку</h3>
            <form id="edit-grade-form">
                <input type="hidden" id="edit-grade-id" value="${grade.id}">
                <div>
                    <label>Студент:</label>
                    <select id="edit-grade-student-id" required></select>
                </div>
                <div>
                    <label>Предмет:</label>
                    <input type="text" id="edit-grade-subject" value="${grade.subject}" required>
                </div>
                <div>
                    <label>Оценка (1-5):</label>
                    <input type="number" id="edit-grade-value" value="${grade.grade}" min="1" max="5" required>
                </div>
                <div>
                    <label>Семестр:</label>
                    <input type="number" id="edit-grade-semester" value="${grade.semester}" required>
                </div>
                <div>
                    <label>Посещаемость (%):</label>
                    <input type="number" id="edit-grade-attendance" value="${grade.attendance_percent}" min="0" max="100" step="0.1" required>
                </div>
                <div>
                    <label>Выполнение заданий (%):</label>
                    <input type="number" id="edit-grade-assignment" value="${grade.assignment_completion}" min="0" max="100" step="0.1" required>
                </div>
                <div>
                    <label>Результат экзамена (1-5):</label>
                    <input type="number" id="edit-grade-exam" value="${grade.exam_result || 0}" min="1" max="5" required>
                </div>
                <div class="form-buttons">
                    <button type="submit">Сохранить</button>
                    <button type="button" onclick="closeModal('edit-grade-modal')">Отмена</button>
                </div>
            </form>
        </div>
    `;
    document.body.appendChild(modal);
    modal.style.display = 'block';
    
    // Заполнить селект студентов
    const select = document.getElementById('edit-grade-student-id');
    students.forEach(student => {
        const option = document.createElement('option');
        option.value = student.id;
        option.textContent = `${student.name} ${student.surname}`;
        if (student.id === grade.student_id) option.selected = true;
        select.appendChild(option);
    });
    
    document.getElementById('edit-grade-form').addEventListener('submit', async (e) => {
        e.preventDefault();
        const formData = new URLSearchParams();
        formData.append('session_id', sessionId);
        formData.append('id', document.getElementById('edit-grade-id').value);
        formData.append('student_id', document.getElementById('edit-grade-student-id').value);
        formData.append('subject', document.getElementById('edit-grade-subject').value);
        formData.append('grade', document.getElementById('edit-grade-value').value);
        formData.append('semester', document.getElementById('edit-grade-semester').value);
        formData.append('attendance', document.getElementById('edit-grade-attendance').value);
        formData.append('assignment', document.getElementById('edit-grade-assignment').value);
        formData.append('exam_result', document.getElementById('edit-grade-exam').value);
        
        try {
            const response = await fetch('/api/admin/grades/update', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: formData
            });
            
            const data = await response.json();
            if (data.success) {
                closeModal('edit-grade-modal');
                loadAdminData();
            } else {
                alert('Ошибка обновления');
            }
        } catch (error) {
            alert('Ошибка соединения');
        }
    });
}

function closeModal(modalId) {
    const modal = document.getElementById(modalId);
    if (modal) {
        modal.remove();
    }
}

