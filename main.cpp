//#include <QCoreApplication>

#include <QtCore>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDebug>
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <iostream>
#include <regex>

using namespace std;

const string input = "sdweqwP:2222/33:PqweqwA:0.2:AqwqT:39.4:Tqweqw";

class sensorData {
public:
    void setPresure(string newPresure) {
        presure = newPresure;
    }
    void setAlcohol(string newAlcohol) {
        alcohol = newAlcohol;
    }
    void setTemperature(string newTemperature) {
        temperature = newTemperature;
    }
    string getPresure() {
        return presure;
    }
    string getAlcohol() {
        return alcohol;
    }
    string getTemperature() {
        return temperature;
    }

private:
    string presure;
    string alcohol;
    string temperature;
};

void extractData(string data, sensorData& medData) {

    regex presurePattern("P:(.*?)\\:P");
    regex alcoholPattern("A:(.*?)\\:A");
    regex temperaturePattern("T:(.*?)\\:T");

    smatch match;

    if (regex_search(data, match, presurePattern)) {medData.setPresure(match.str(1));}
    if (regex_search(data, match, alcoholPattern)) {medData.setAlcohol(match.str(1));}
    if (regex_search(data, match, temperaturePattern)) {medData.setTemperature(match.str(1));}
}

void createJSONFile(const QString& filename, const QString& presure, const QString& alcohol, const QString& temperature) {
    QJsonObject medicalData;
    medicalData["presure"] = presure;
    medicalData["alcohol"] = alcohol;
    medicalData["temperature"] = temperature;

    QJsonDocument doc(medicalData);

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning("Failed to open file: %s", file.errorString().toUtf8().constData());
        return;
    }

    if (file.write(doc.toJson()) == -1) {
        qWarning("Failed to write to file: %s", file.errorString().toUtf8().constData());
        return;
    }

    file.close();

    qDebug() << "JSON successfully created!";
}

void uploadToDB(QString presure, QString alcohol, QString temperature) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("medicalData.db");

    if (!db.open()) {
        qWarning() << "Error connectiong to database: " << db.lastError().text();
        return;
    }

    QSqlQuery query;
    if (!query.exec("CREATE TABLE IF NOT EXISTS medicaldata (id INTEGER PRIMARY KEY AUTOINCREMENT, presure TEXT, alcohol TEXT, temperature TEXT)")) {
        qWarning() << "Error creating table: " << query.lastError().text();
        return;
    }

    query.prepare("INSERT INTO medicaldata (presure, alcohol, temperature) VALUES (?,?,?)");
    query.addBindValue(presure);
    query.addBindValue(alcohol);
    query.addBindValue(temperature);

    if (!query.exec()) {
        qWarning() << "Error inserting data: " << query.lastError().text();
        return;
    }
}

void selectFromDB() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("medicalData.db");

    if (!db.open()) {
        qWarning() << "Error connectiong to database: " << db.lastError().text();
        return;
    }

    QSqlQuery query;

    if(!query.exec("SELECT * FROM medicaldata")){
        qWarning() << "Error selecting from database: " << db.lastError().text();
    }
    while (query.next()) {
        int id = query.value(0).toInt();
        QString presure = query.value(1).toString();
        QString alcohol = query.value(2).toString();
        QString temperature = query.value(3).toString();

        qDebug() << "#: " << id
                 << "presure: " << presure
                 << "alcohol: " << alcohol
                 << "temperature: " << temperature;
    }
    db.close();
}

void analyzeData(QString presure, QString alcohol, QString temperature) {
    QStringList presureParts = presure.split('/');
    int systolicPressure = presureParts[0].toInt();
    int diastolicPressure = presureParts[1].toInt();
    float alcoholNum = alcohol.toFloat();
    float temperatureNum = temperature.toFloat();


    if (systolicPressure > 130 || diastolicPressure > 85) {
        qDebug() << "You have high arterial pressure";
    } else if (systolicPressure < 110 || diastolicPressure < 70) {
        qDebug() << "You have low arterial pressure";
    } else {
        qDebug() << "You have normal arterial pressure";
    }

    if (alcoholNum > 0.16) {
        qDebug() << "You have high alcohol % in blood";
    } else {
        qDebug() << "You have normal alcohol % in blood";
    }

    if (temperatureNum > 38.1) {
        qDebug() << "You have high temperature";
    } else {
        qDebug() << "You have normal temperature";
    }

}

QString convertString(string in) {return QString::fromStdString(in);}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    sensorData medData;

    extractData(input, medData);

    cout << "Pressure: " << medData.getPresure()<< endl
         << "Alcohol: " << medData.getAlcohol()<< endl
         << "Temperature: " << medData.getTemperature() << endl << endl;

    createJSONFile("medicalData.json", convertString(medData.getPresure()), convertString(medData.getAlcohol()), convertString(medData.getTemperature()));
    uploadToDB(convertString(medData.getPresure()), convertString(medData.getAlcohol()), convertString(medData.getTemperature()));
    selectFromDB();
    analyzeData(convertString(medData.getPresure()), convertString(medData.getAlcohol()), convertString(medData.getTemperature()));
    return app.exec();
}
