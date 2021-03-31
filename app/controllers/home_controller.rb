class HomeController < ApplicationController
    require 'zinnia'

    def top
    end

    def classify
        char_data = params[:char_data]
  
        s = Zinnia::Character.new
        r = Zinnia::Recognizer.new
        r.open("/usr/local/lib/zinnia/model/tomoe/handwriting-ja.model")

        if (!s.parse(char_data))
            return render :status => 500
        end
        
        result = r.classify(s, 10)
        size = result.size()
        result_json = []
        size.times { |i|
            result_json << [result.value(i).force_encoding("UTF-8"), result.score(i)]
        }

        render :json => {result: result_json}.to_json, :status => 200 
    end
end
