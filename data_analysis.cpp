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
std::unordered_map<std::string, std::unordered_map<std::string, size_t>> doc_word_freq; // слова в каждом документе 
std::unordered_map<std::string, size_t> word_freq; // слова во всех документах
std::set<std::string> unique_words;
std::unordered_map<std::string, size_t> count_of_words; // количество слов в документе
std::vector<std::string> file_names;
std::unordered_map<std::string, std::unordered_set<std::string>> count_of_docs_with_word; // df
std::unordered_map<std::string, std::unordered_set<std::string>> unique_words_in_docs;

bool valide_word(const std::string& word){
    return count_of_docs_with_word.find(word) == count_of_docs_with_word.end();
}

bool valide_file_name(const std::string& file_name){
    return doc_word_freq.find(file_name) == doc_word_freq.end();
}

const std::string& normalise(const std::string& word){
    std::string& normalized_word = const_cast<std::string&>(word);

    std::transform(normalized_word.begin(), normalized_word.end(), normalized_word.begin(),
        [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    normalized_word.erase(std::remove_if(normalized_word.begin(), normalized_word.end(),
        [](unsigned char c){ return !std::isalpha(c); }), normalized_word.end());

    return const_cast<const std::string&>(normalized_word);
}

double tf(const std::string& file_name, const std::string& word){
    return static_cast<double>(doc_word_freq.at(file_name)[word]) / static_cast<double>(count_of_words.at(file_name));
}

double idf(const std::string& word){
    return std::log(static_cast<double>(file_names.size()) / static_cast<double>(count_of_docs_with_word[word].size()));
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
    };

    if (raw_string.empty()){
        std::cout << "WARNING: [DOCUMENT]: The document \"" << file_name << "\" is empty\n";
        return;
    }

    std::transform(raw_string.begin(), raw_string.end(), raw_string.begin(),
        [](unsigned char c){ return static_cast<char>(std::tolower(c)); }); // переделать в обработку юникод

    std::replace_if(raw_string.begin(), raw_string.end(),
        [](unsigned char c){ return !std::isalpha(c); }, ' ');

    std::stringstream iss(raw_string);

    std::vector<std::string> words{
        std::istream_iterator<std::string>(iss),
        std::istream_iterator<std::string>()
    };

    std::unordered_map<std::string, size_t> word_freq_in_doc;
    std::unordered_set<std::string> unique_words_in_doc;


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

    unique_words_in_docs[file_name] = unique_words_in_doc;

}

void word(const std::string& word){
    const std::string normalised_word = normalise(word);
    
    if (valide_word(normalised_word)){
        throw std::runtime_error("ERROR: [WORD]: Word \"" + word + "\" was not found\n");
    }

    std::cout << "Word: " << normalised_word << "\n" << "Documents total: " << count_of_docs_with_word.at(normalised_word).size()
    << "\n" << "IDF: " << idf(normalised_word) << "\n";

    int index = 1;

    std::for_each(count_of_docs_with_word.at(normalised_word).begin(), count_of_docs_with_word.at(normalised_word).end(),
        [&normalised_word, &index](const auto& file_name){
        std::cout << index << ". " << file_name << " (TF = " << tf(file_name, normalised_word) << ")\n";
        ++index;
    });   
}

void word_in_doc(const std::string& word, const std::string& file_name){
    if (valide_file_name(file_name)){
        throw std::runtime_error("ERROR: [DOCUMENT]: Document \"" + file_name + "\" was not found\n");
    }

    const std::string normalised_word = normalise(word);

    if (valide_word(normalised_word)){
        throw std::runtime_error("ERROR: [WORD]: Word \"" + word + "\" was not found\n");
    }

    if (doc_word_freq.at(file_name).find(normalised_word) == doc_word_freq.at(file_name).end()){
        throw std::runtime_error("ERROR: [WORD]: Word \"" + word + "\" was not found in document \"" + file_name + "\"\n");
    }

    std::cout << "Word: " << normalised_word << "\n" << "Document: " << file_name << "\n"
            << "Count: " << doc_word_freq.at(file_name).at(normalised_word) << "\n" << "TF: " << tf(file_name, normalised_word)
            << "\n" << "TF-IDF: " << tf_idf(file_name, normalised_word) << "\n";
}

void doc(const std::string& file_name){
    if (valide_file_name(file_name)){
        throw std::runtime_error("ERROR: [DOCUMENT]: Document \"" + file_name + "\" was not found\n");
    }

    std::cout << "Document: " << file_name << "\n" 
            << "Total words: " <<  count_of_words.at(file_name) << "\n" << "Unique words: "
            << unique_words_in_docs.at(file_name).size() << "\nTop words:\n";

    std::vector<std::pair<std::string, uint16_t>> sorted_words(doc_word_freq.at(file_name).begin(), doc_word_freq.at(file_name).end());
    std::sort(sorted_words.begin(), sorted_words.end(), [](const auto& a, const auto& b){
        if (a.second != b.second){
            return a.second > b.second; 
        }
        return a.first < b.first; 
    });
    
    size_t count = std::min(static_cast<size_t>(5), sorted_words.size());
    if (count == 0){
        throw std::runtime_error("ERROR [DOCUMENT]: The document \"" + file_name + "\" is empty");
    }
    int index = 1;

    std::for_each(sorted_words.begin(), std::next(sorted_words.begin(), count), [file_name, &index](const auto& pair){
        std::cout << index << ". " << pair.first << " : (TF = " << tf(file_name, pair.first) << ", IDF = " << idf(pair.first) << ", TF-IDF = " << tf_idf(file_name, pair.first)<< ")\n";
        ++index;
    });
}

double sum(std::vector<std::string> v, const std::string& file_name){
    double sum = 0;
    std::for_each(v.begin(), v.end(), [file_name, &sum](std::string& word){sum += tf_idf(file_name, word);});
    return sum;
}

void query(int argc, char** argv){
    std::vector<std::string> phrase(argv + 2, argv + argc);
    std::vector<std::string> normalised_phrase;
    std::transform(phrase.begin(), phrase.end(), phrase.begin(), 
            [normalised_phrase](const std::string& word){return const_cast<std::string&>(normalise(word));});

    std::cout << "Query: ";
    std::for_each(phrase.begin(), phrase.end(),
        [](const std::string& word){std::cout << word  << " ";});

    std::cout << "\nResults:\n";

    int index = 1;

    std::for_each(file_names.begin(), file_names.end(),
        [phrase, &index](const std::string& file_name){
            std::cout << index << ". " << file_name  << ": (" << sum(phrase, file_name) << ")\n";
            ++index;
        });

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
        doc(std::string(argv[2]));
    } else if (std::string(argv[1]) == "QUERY"){
        if (argc < 3) throw std::runtime_error("ERRROR [INPUT]: Not enough arguments"); 
        query(argc, argv);
    } else {
        throw std::runtime_error("ERRROR [INPUT]: Unknown command");
    }

    return 0;
}
