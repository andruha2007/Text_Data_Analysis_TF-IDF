#include <iostream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <unordered_map>
#include <set>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <cstdint>
#include <cmath>
#include <unordered_set>


// это пока что выглядит плохо
std::unordered_map<std::string, std::unordered_map<std::string, uint16_t>> doc_word_freq; // слова в каждом документе 
std::unordered_map<std::string, uint16_t> word_freq; // слова во всех документах
std::set<std::string> unique_words;
std::unordered_map<std::string, size_t> count_of_words; // количество слов в документе
std::vector<std::string> file_names;
std::unordered_map<std::string, std::unordered_set<std::string>> count_of_docs_with_word; // df

double tf(const std::string& file_name, const std::string& word){
    return static_cast<double>(doc_word_freq.at(file_name).at(word)) / static_cast<double>(count_of_words.at(file_name));
}

double idf(const std::string& word){
    return std::log(static_cast<double>(file_names.size()) / static_cast<double>(count_of_docs_with_word.at(word).size()));
}

double tf_idf(const std::string& file_name, const std::string& word){
    return tf(file_name, word) * idf(word);
}

void fill_statistics(const std::string& file_name){
    std::ifstream file(file_name);

    if (!file.is_open()){
        std::cout << "WARNING: [DOCUMENT]: Cannot open document \"" << file_name << "\"\n";
        return;
    }

    std::string raw_string{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    }; // чтение файла в отдельную функцию

    if (raw_string.empty()){
        std::cout << "WARNING: [DOCUMENT]: The document \"" << file_name << "\" is empty\n";
        return;
    }

    std::transform(raw_string.begin(), raw_string.end(), raw_string.begin(),
        [](unsigned char c){ return static_cast<char>(std::tolower(c)); }); // переделать в обработку юникод

    std::replace_if(raw_string.begin(), raw_string.end(),
        [](unsigned char c){ return !std::isalpha(c); }, ' '); // сохранять дефисы и апострафы

    std::stringstream iss(raw_string);

    std::vector<std::string> words{
        std::istream_iterator<std::string>(iss),
        std::istream_iterator<std::string>()
    };

    std::unordered_map<std::string, uint16_t> word_freq_in_doc;
    std::set<std::string> unique_words_in_doc;

    std::for_each(words.begin(), words.end(), [&word_freq_in_doc, &unique_words_in_doc, file_name](const std::string& word){
        ++word_freq_in_doc[word];
        ++word_freq[word];
        unique_words.insert(word);
        unique_words_in_doc.insert(word);
        ++count_of_words[file_name];
    });

    std::for_each(unique_words_in_doc.begin(), unique_words_in_doc.end(), [file_name](const std::string& word){
        count_of_docs_with_word[word].insert(file_name);
    });

    doc_word_freq[file_name] = word_freq_in_doc; 

}


void word_in_doc(const std::string& word, const std::string& file_name){
    if (doc_word_freq.find(file_name) == doc_word_freq.end()){ // сделать общий валидатор входных аргументов
        std::cout << "WARNING: [DOCUMENT]: Document \"" << file_name << "\" was not found\n";
        return;
    }

    if (doc_word_freq.at(file_name).find(word) == doc_word_freq.at(file_name).end()){ //  нормализовать входное слово так же, как слова из файла
        std::cout << "WARNING: [WORD]: Word \"" << word << "\" was not found in document \"" << file_name << "\"\n";
        return;
    }

    std::cout << "Word: " << word << "\n" << "Document: " << file_name << "\n" 
            << "Count: " << doc_word_freq.at(file_name).at(word) << "\n" << "TF: " << tf(file_name, word) 
            << "\n" << "TF-IDF: " << tf_idf(file_name, word) << "\n"; 
}

void word(const std::string& word){
    if (count_of_docs_with_word.find(word) == count_of_docs_with_word.end()){
        std::cout << "WARNING: [WORD]: Word \"" << word << "\" was not found\n";
        return;
    }

    std::cout << "Word: " << word << "\n" << "Documents total: " << count_of_docs_with_word.at(word).size() 
    << "\n" << "IDF: " << idf(word) << "\n";
    std::copy(count_of_docs_with_word.at(word).begin(), count_of_docs_with_word.at(word).end(), 
            std::ostream_iterator<std::string>(std::cout, "\n"));
}

int main(int argc, char** argv){
    if (argc < 2) throw std::runtime_error("ERRROR [INPUT]: Not enough arguments");

    std::ifstream file("documents.txt");

    file_names = std::vector<std::string>{
        std::istream_iterator<std::string>(file),
        std::istream_iterator<std::string>()
    };

    std::for_each(file_names.begin(), file_names.end(), fill_statistics);

    //std::copy(file_names.begin(), file_names.end(), std::ostream_iterator<std::string>(std::cout, "\n"));

    if (std::string(argv[1]) == "WORD"){
        if (argc < 3) throw std::runtime_error("ERRROR [INPUT]: Not enough arguments");
            word(std::string(argv[2]));
    } else if (std::string(argv[1]) == "WORD_IN_DOC"){
        if (argc < 4) throw std::runtime_error("ERRROR [INPUT]: Not enough arguments"); 
            word_in_doc(std::string(argv[2]), std::string(argv[3])); 

    } else if (std::string(argv[1]) == "DOC"){
        if (argc < 3) throw std::runtime_error("ERRROR [INPUT]: Not enough arguments"); 
        
    } else if (std::string(argv[1]) == "QUERY"){
        if (argc < 3) throw std::runtime_error("ERRROR [INPUT]: Not enough arguments"); 
        
    } else {
        throw std::runtime_error("ERRROR [INPUT]: Unknown command");
    }

    return 0;
}
