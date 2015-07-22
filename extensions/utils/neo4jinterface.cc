#include "neo4jinterface.h"
#include <jsoncpp/json/json.h>

std::stringstream Neo4jInterface::recv_data;

Neo4jInterface::Neo4jInterface()
{
}

size_t Neo4jInterface::curlCallback(void *contents, size_t size, size_t nmemb, void *stream)
{
    for (int c = 0; c<size*nmemb; c++)
    {
        recv_data << ((char*)contents)[c];
    }

    return size*nmemb;
}

std::string Neo4jInterface::PerformNeo4jTrx(std::vector<std::string> statementsStr)
{
    Json::Value neo4jTrx = Json::Value(Json::objectValue);
    Json::Value  statements = Json::Value(Json::arrayValue);

    for (int i = 0; i < statementsStr.size(); i++)
    {
        Json::Value statementObject = Json::Value(Json::objectValue);
        statementObject["statement"] = statementsStr.at(i);
        statements.append(statementObject);
    }

    neo4jTrx["statements"] = statements;

    Json::StyledWriter writer;

    struct curl_slist *headers = NULL;
    if ((ch = curl_easy_init()) == NULL)
    {
        fprintf(stderr, "ERROR: Failed to create curl handle");
        return "";
    }

    headers = curl_slist_append(headers, "Accept: application/json; charset=UTF-8");
    headers = curl_slist_append(headers, "Content-type: application/json");

    curl_easy_setopt(ch, CURLOPT_URL, "http://10.0.2.2:7474/db/data/transaction/commit");
    curl_easy_setopt(ch, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(ch, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(ch, CURLOPT_POSTFIELDS, writer.write(neo4jTrx).c_str());
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, curlCallback);

    int rcode = curl_easy_perform(ch);

    /*
    if(rcode != CURLE_OK)
          fprintf(stderr, "curl_easy_perform() failed: %s\n",
                  curl_easy_strerror(rcode));
    */
    /* always cleanup */
    curl_easy_cleanup(ch);

    std::string ret(recv_data.str());
    recv_data.str("");

    return ret;
}

std::string Neo4jInterface::PerformNeo4jTrx(std::string statement)
{
    Json::Value neo4jTrx = Json::Value(Json::objectValue);
    Json::Value  statements = Json::Value(Json::arrayValue);

    Json::Value statementObject = Json::Value(Json::objectValue);

    statementObject["statement"] = statement;

    statements.append(statementObject);

    neo4jTrx["statements"] = statements;

    Json::StyledWriter writer;

    struct curl_slist *headers = NULL;
    if ((ch = curl_easy_init()) == NULL)
    {
        fprintf(stderr, "ERROR: Failed to create curl handle");
        return "";
    }

    headers = curl_slist_append(headers, "Accept: application/json; charset=UTF-8");
    headers = curl_slist_append(headers, "Content-type: application/json");

    curl_easy_setopt(ch, CURLOPT_URL, "http://10.0.2.2:7474/db/data/transaction/commit");
    curl_easy_setopt(ch, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(ch, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(ch, CURLOPT_POSTFIELDS, writer.write(neo4jTrx).c_str());
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, curlCallback);

    int rcode = curl_easy_perform(ch);

    /*
    if(rcode != CURLE_OK)
          fprintf(stderr, "curl_easy_perform() failed: %s\n",
                  curl_easy_strerror(rcode));
    */
    /* always cleanup */
    curl_easy_cleanup(ch);

    std::string ret(recv_data.str());
    recv_data.str("");

    return ret;
}
