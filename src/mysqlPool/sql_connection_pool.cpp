#include "../include/mysqlPool/sql_connection_pool.h"
#include "../include/Log/log.h"

ConnectionPool::ConnectionPool():m_CurConn(0), m_FreeConn(0) {

}

ConnectionPool::~ConnectionPool() {
    destroyPool();
}

void ConnectionPool::init(const std::string &url, const std::string &User, const std::string &PassWord,
                          const std::string &DataBaseName, int Port, int MaxConn, int close_log) {
    m_url = url;
    m_User = User;
    m_PassWord = PassWord;
    m_DatabaseName = DataBaseName;
    m_Port = Port;
    m_close_log = close_log;

    // 初始化数据库连接池
    for (int i = 0; i < MaxConn; i++) {
        MYSQL *con = mysql_init(nullptr);
        if (!con) {
            LOG_ERROR("MySQL Init Error");
            throw std::exception();
        }
        con = mysql_real_connect(con,
                                 url.c_str(),
                                 User.c_str(),
                                 PassWord.c_str(),
                                 DataBaseName.c_str(),
                                 Port,
                                 nullptr,
                                 0);
        if (!con) {
            LOG_ERROR("MySQL Connect Error: %s", mysql_error(con));
            destroyPool(); // 清理已创建的资源
            throw std::exception();
        }

        m_connList.push_back(con);
        m_FreeConn++;
    }
    m_MaxConn = m_FreeConn;
    LOG_INFO("Connection Pool Initialized. Max Connections: %d", m_MaxConn);
}

// 单例模式，数据库连接池
ConnectionPool *ConnectionPool::getInstance() {
    static ConnectionPool connPool;
    return &connPool;
}

// 获取数据库连接
MYSQL *ConnectionPool::getConnection() {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_connList.empty()) {
        m_cond.wait(lock);
    }

    if (m_connList.empty()) {
        LOG_ERROR("No available connections in the pool!");
        return nullptr;
    }

    MYSQL *con = m_connList.front();
    m_connList.pop_front();
    m_FreeConn--;
    m_CurConn++;
    LOG_INFO("Connection acquired. Free: %d, Current: %d", m_FreeConn, m_CurConn);
    return con;
}

bool ConnectionPool::releaseConnection(MYSQL *conn) {
    if (!conn) return false;
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_CurConn <= 0) {
        LOG_ERROR("Invalid connection release attempt!");
        return false;
    }
    m_connList.push_back(conn);
    m_FreeConn++;
    m_CurConn--;
    m_cond.notify_one(); // 有空闲的资源，唤醒等待线程
    LOG_INFO("Connection released. Free: %d, Current: %d", m_FreeConn, m_CurConn);
    return true;
}

void ConnectionPool::destroyPool() {
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto conn : m_connList) {
        mysql_close(conn);
    }
    m_connList.clear();
    m_FreeConn = 0;
    m_CurConn = 0;
    LOG_INFO("Connection pool destroyed.");
}

int ConnectionPool::getFreeConn() {
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_FreeConn;
}

// con 可以存储一个 MYSQL* 类型的指针的地址
ConnectionRAII::ConnectionRAII(MYSQL **con, ConnectionPool *connPool) {
    *con = connPool->getConnection();
    if (*con == nullptr) {
        LOG_ERROR("Failed to acquire connection from pool!");
        throw std::runtime_error("No available connections");
    }
    conRAII = *con;
    poolRAII = connPool;
}

ConnectionRAII::~ConnectionRAII() {
    poolRAII->releaseConnection(conRAII);
}
