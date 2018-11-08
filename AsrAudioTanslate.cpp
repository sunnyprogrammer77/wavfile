#include "AsrAudioTanslate.h"

AsrAudioTanslate::AsrAudioTanslate()
{
	audio_file = NULL;
	m_InterceptLen = 200000;
	m_AcountInfoType = "AccountInfo";

}
AsrAudioTanslate::~AsrAudioTanslate()
{
	if (account_info)
		 account_info->ReleaseInstance();
}
bool AsrAudioTanslate::InitAccountInfo()
{
	string account_info_file;
	account_info = AccountInfo::GetInstance();
	account_info_file = account_info_file+ "testdata/" + m_AcountInfoType + ".txt";
	bool account_success = account_info->LoadFromFile(account_info_file);
	//bool account_success = account_info->LoadFromCode();
	if (!account_success)
	{
		printf("AccountInfo read from %s failed\n", account_info_file.c_str());
		getchar();
		return false;
	}
	printf("AccountInfo Read success\n");
	return true;
}
bool AsrAudioTanslate::HciInit()
{
	HCI_ERR_CODE err_code = HCI_ERR_NONE;
	//���ô�����"�ֶ�=ֵ"����ʽ������һ���ַ���������ֶ�֮����','�������ֶ������ִ�Сд��
	string init_config = "";
	init_config += "appKey=" + account_info->app_key();              //����Ӧ�����
	init_config += ",developerKey=" + account_info->developer_key(); //���ƿ�������Կ
	init_config += ",cloudUrl=" + account_info->cloud_url();         //�����Ʒ���Ľӿڵ�ַ
	init_config += ",authpath=" + account_info->auth_path();         //��Ȩ�ļ�����·������֤��д
	init_config += ",logfilepath=" + account_info->logfile_path();   //��־��·��
	init_config += ",loglevel=5";								     //��־�ĵȼ�
	init_config += ",logfilesize=512";								 //��־�ļ��Ĵ�С
	//��������ʹ��Ĭ��ֵ��������ӣ���������ÿ��Բο������ֲ�
	err_code = hci_init(init_config.c_str());
	if (err_code != HCI_ERR_NONE)
	{
		printf("hci_init return (%d:%s)\n", err_code, hci_get_error_info(err_code));
		getchar();
		return false;
	}
	printf("hci_init success\n");
	return true;
}
bool AsrAudioTanslate::HciAsrInit()
{
	HCI_ERR_CODE err_code = HCI_ERR_NONE;
	asr_init_config += "initcapkeys=" + account_info->cap_key();                    //������initcapkeys�������Ҫʹ�õ�������������ߵ�һ��ʶ���Ч��
	asr_init_config += ",datapath=" + account_info->data_path();                    //����Ǳ����������˴�ָ����������������Դ
	err_code = hci_asr_init(asr_init_config.c_str());
	if (err_code != HCI_ERR_NONE)
	{
		printf("hci_asr_init return (%d:%s) \n", err_code, hci_get_error_info(err_code));
		return false;
	}
	printf("hci_asr_init success \n");
	return true;
}
void AsrAudioTanslate::SetInterceptLen(size_t len)
{
	m_InterceptLen = len;
}
void AsrAudioTanslate::SetAcountInfoTyp(const char* Type)
{
	m_AcountInfoType = Type;
}
void AsrAudioTanslate::CloseAsr()
{
	hci_asr_release();
}
void AsrAudioTanslate::CloseHci()
{
	hci_release();
}
bool AsrAudioTanslate::StartRecog(const char* audiofile,size_t buffer, bool fileflag, const char* outfile)
{
	audio_file = audiofile;
	if (fileflag)
		return Recog(account_info->cap_key(), asr_init_config, audio_file, outfile, buffer);
	else
		return RealtimeRecog(account_info->cap_key(), asr_init_config, audio_file, outfile, buffer);
}
bool AsrAudioTanslate::Recog(const string &cap_key, const string& recog_config, const char* audio_file, const char * file_name, size_t buffer)
{
	HCI_ERR_CODE err_code = HCI_ERR_NONE;
	// �������������ļ�
	HciExampleComon::FileReader voiceData;
	if (voiceData.Load(audio_file) == false)
	{
		printf("Open input voice file %s error!\n", audio_file);
		return false;
	}

	// ���� ASR Session
	int nSessionId = -1;
	string strSessionConfig = "capkey=" + cap_key;
	strSessionConfig += "," + recog_config;
	//�˴�Ҳ���Դ����������ã��μ������ֲᣬ�˴��������ò���Ĭ��ֵ
	printf("hci_asr_session_start config[ %s ]\n", strSessionConfig.c_str());
	err_code = hci_asr_session_start(strSessionConfig.c_str(), &nSessionId);
	if (err_code != HCI_ERR_NONE)
	{
		printf("hci_asr_session_start return (%d:%s)\n", err_code, hci_get_error_info(err_code));
		return false;
	}
	printf("hci_asr_session_start success\n");

	// ʶ��
	ASR_RECOG_RESULT asrResult;
	printf("hci_asr_recog config[ %s ]\n", recog_config.c_str());
	//��ȡ��Ƶ�ļ�����
	int voice_Length = voiceData.buff_len_;//�����ļ�����
	int nlen = 0;
	int m_length = 0;
	std::ofstream ofs(file_name);
	if (!ofs)
	{
		printf("�ļ���ʧ��!");
		ofs.close();
		return false;
	}
	while (nlen < voice_Length)
	{
		if (voice_Length - nlen > m_InterceptLen)
		{
			m_length = m_InterceptLen;
		}
		else
		{
			m_length = voice_Length - nlen;
		}
		err_code = hci_asr_recog(nSessionId, voiceData.buff_ + buffer+ nlen, m_length, NULL, NULL, &asrResult);//voiceData.buff_len_
		printf("%d", voiceData.buff_len_);

		if (err_code == HCI_ERR_NONE)
		{
			printf("hci_asr_recog success\n");
			// ���ʶ����
			//PrintAsrResult(asrResult);

			HciExampleComon::SetSpecialConsoleTextAttribute();
			printf("\nrecog result:\n");
			for (int index = 0; index < (int)asrResult.uiResultItemCount; ++index)
			{
				ASR_RECOG_RESULT_ITEM& item = asrResult.psResultItemList[index];
				ofs.write(item.pszResult, strlen(item.pszResult));
				ofs.write("\n", strlen("\n"));
				printf("index:%d\t", index);
				printf("text:");
				HciExampleComon::PrintUtf8String(item.pszResult);
				printf("\tscore:%d\n", item.uiScore);
			}
			printf("\n");
			HciExampleComon::SetOriginalConsoleTextAttribute();




			// �ͷ�ʶ����
			hci_asr_free_recog_result(&asrResult);
		}
		else
		{
			printf("hci_asr_recog return (%d:%s)\n", err_code, hci_get_error_info(err_code));
			break;
		}
		nlen += m_length;
	}
	ofs.close();
	// ��ֹ ASR Session
	hci_asr_session_stop(nSessionId);
	printf("hci_asr_session_stop\n");
	return true;
}

bool AsrAudioTanslate::RealtimeRecog(const string &cap_key, const string& recog_config, const char* audio_file, const char* outfile, size_t buffer)
{
	HCI_ERR_CODE errCode = HCI_ERR_NONE;
	// ������Ƶ����
	HciExampleComon::FileReader voiceData;
	if (voiceData.Load(audio_file) == false)
	{
		printf("Open input voice file %s error!\n", audio_file);
		return false;
	}

	// ���� ASR Session
	int nSessionId = -1;
	string strSessionConfig = "vadswitch=no,vadtail=1234,vadseg=5678,realtime=yes,capkey=" + cap_key;
	strSessionConfig += "," + recog_config;
	//�˴�Ҳ���Դ����������ã��μ������ֲᣬ�˴��������ò���Ĭ��ֵ
	printf("hci_asr_session_start config[ %s ]\n", strSessionConfig.c_str());
	errCode = hci_asr_session_start(strSessionConfig.c_str(), &nSessionId);
	if (errCode != HCI_ERR_NONE)
	{
		printf("hci_asr_session_start return (%d:%s)\n", errCode, hci_get_error_info(errCode));
		return false;
	}
	printf("hci_asr_session_start success\n");

	// ʵʱʶ�����ģ�⣬����ʶ����Ƶ���ݷ�Ϊ��Σ���δ���ʶ��ӿ�
	// ��ĳ�δ�����Ƶ���ݺ��⵽ĩ�ˣ�������ѭ������ֹ��Ƶ���ݴ�����׼����ȡʶ����
	printf("hci_asr_recog config[ %s ]\n", recog_config.c_str());
	int nPerLen = m_InterceptLen;	// 0.2s
	int nLen = 0;            // ��ǰ�Ѵ���ĳ���
	ASR_RECOG_RESULT asrResult;
	while (nLen < voiceData.buff_len_)
	{
		// ����Ҫ����Ĳ���ʶ������ݳ��ȣ�
		// ʣ��Ķ���nPerLen����nPerLen���ֽڣ�����������ʣ������
		int nThisLen = 0;
		if (voiceData.buff_len_ - nLen >= nPerLen)
		{
			nThisLen = nPerLen;
		}
		else
		{
			nThisLen = voiceData.buff_len_ - nLen;
		}

		errCode = hci_asr_recog(nSessionId, voiceData.buff_ +buffer + nLen, nThisLen, NULL, NULL, &asrResult);
		if (asrResult.uiResultItemCount > 0)
		{
			//PrintAsrResult(asrResult);
			hci_asr_free_recog_result(&asrResult);
		}

		if (errCode == HCI_ERR_ASR_REALTIME_END)
		{
			errCode = hci_asr_recog(nSessionId, NULL, 0, NULL, NULL, &asrResult);
			if (errCode == HCI_ERR_NONE)
			{
				printf("hci_asr_recog success\n");
				// ���ʶ����
				//PrintAsrResult(asrResult);
				// �ͷ�ʶ����
				hci_asr_free_recog_result(&asrResult);
			}
			else
			{
				printf("hci_asr_recog return (%d:%s)\n", errCode, hci_get_error_info(errCode));
				break;
			}
		}
		else if (errCode == HCI_ERR_ASR_REALTIME_WAITING || errCode == HCI_ERR_ASR_REALTIME_NO_VOICE_INPUT)
		{
			//������ʶ��ĳ������������������������ʶ��������Ƶ��
			//HCI_ERR_ASR_REALTIME_WAITING ��ʵʱʶ��ȴ���Ƶ�������ǣ���û�����ݣ���������Ҫ�������ݡ�
			//HCI_ERR_ASR_REALTIME_NO_VOICE_INPUT �����ǣ�û�м�⵽��Ƶ��㣬��������vadHead�ķ�Χ����ʱ�������豸��������״̬��

			printf("hci_asr_recog return (%d:%s)\n", errCode, hci_get_error_info(errCode));

			nLen += nThisLen;
		}
		else
		{
			//ʶ��ʧ��
			printf("hci_asr_recog return (%d	:%s)\n", errCode, hci_get_error_info(errCode));
			break;
		}


		//ģ����ʵ˵�������٣�����200ms���ݺ���ȴ�200ms
#if defined(WIN32) || defined(WIN64)
		Sleep(200);
#else
		//usleep(200 * 1000);
#endif
	}

	// ��δ��⵽�˵㣬�������Ѿ�������ϣ�����Ҫ�������������������
	// ���߼�⵽ĩ���ˣ�Ҳ��Ҫ�������棬��ȡ���
	if (errCode == HCI_ERR_ASR_REALTIME_WAITING || errCode == HCI_ERR_ASR_REALTIME_END)
	{
		errCode = hci_asr_recog(nSessionId, NULL, 0, NULL, NULL, &asrResult);
		if (errCode == HCI_ERR_NONE)
		{
			printf("hci_asr_recog success\n");
			// ���ʶ����
			//PrintAsrResult(asrResult);
			// �ͷ�ʶ����
			hci_asr_free_recog_result(&asrResult);
		}
		else
		{
			printf("hci_asr_recog return (%d:%s)\n", errCode, hci_get_error_info(errCode));
		}
	}

	// ��ֹ ASR Session
	hci_asr_session_stop(nSessionId);
	printf("hci_asr_session_stop\n");
	return true;
}